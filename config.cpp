#include "config.hpp"

#include "ektoplayer.hpp"
#include "filesystem.hpp"
#include "shellsplit.hpp"
#include "common.hpp"
#include "colors.hpp"
#include "xml.hpp"

#include <fstream>
#include <iostream>//XXX
#include <boost/algorithm/string.hpp>

using namespace Ektoplayer;

#define COLUMN_NAMES { "number", "artist", "album", "title", "styles", "bpm", "year" }

static const char * const DEFAULT_PLAYINGINFO_FORMAT_TOP =
  "<text fg='black'>&lt;&lt; </text><title bold='on' fg='yellow' /><text fg='black'> &gt;&gt;</text>";

static const char * const DEFAULT_PLAYINGINFO_FORMAT_BOTTOM = 
  "<artist bold='on' fg='blue' /><text> - </text><album bold='on' fg='red' /><text> (</text><year fg='cyan' /><text>)</text>";

static const char * const DEFAULT_PLAYLIST_FORMAT =
  "<number size='3' fg='magenta' />"
  "<artist rel='25' fg='blue'    />"
  "<album  rel='30' fg='red'     />"
  "<title  rel='33' fg='yellow'  />"
  "<styles rel='20' fg='cyan'    />"
  "<bpm    size='3' fg='green' justify='right' />";

static const char * const DEFAULT_PLAYLIST_FORMAT_256 =
  "<number size='3' fg='97'  />"
  "<artist rel='25' fg='24'  />"
  "<album  rel='30' fg='160' />"
  "<title  rel='33' fg='178' />"
  "<styles rel='20' fg='37'  />"
  "<bpm    size='3' fg='28' justify='right' />";

static const char * const DEFAULT_PLAYINGINFO_FORMAT_TOP_256 =
  "<text fg='236'>&lt;&lt; </text><title bold='on' fg='178' /><text fg='236'> &gt;&gt;</text>";

static const char * const DEFAULT_PLAYINGINFO_FORMAT_BOTTOM_256 = 
 "<artist bold='on' fg='24' /><text> - </text><album bold='on' fg='160' /><text> (</text><year fg='37' /><text>)</text>";


/* === Parsing for primitives === */
int opt_parse_int(const std::string &s) {
  try { 
    size_t pos = 0; 
    int i = std::stoi(s, &pos); // throws on empty
    if (pos != s.length())
      throw pos;
    return i;
  } catch (...) {
    throw std::invalid_argument("not an integer");
  }
}

bool opt_parse_bool(const std::string &s) {
  if (s == "true")        return true;
  else if (s == "false")  return false;
  else throw std::invalid_argument("expected `true` or `false`");
}

char opt_parse_char(const std::string &s) {
  if (s.length() != 1)
    throw std::invalid_argument("expected a single character");
  return s[0];
}
/* === End of primitives === */

/* === Option parsing functions === */
std::string validate_use_colors(const std::string &s) {
  if (s == "auto" || s == "mono" || s == "8" || s == "256")
    return s;

  throw std::invalid_argument("expected auto|mono|8|256");
}

int validate_threads(const std::string &s) {
  int i = opt_parse_int(s);
  if (i < 1)
    throw std::invalid_argument("integer must be > 1");
  return i;
}

std::vector<std::string> opt_parse_tabs_widgets(const std::string &s) {
  std::vector<std::string> widgets;
  boost::split(widgets, s, boost::is_any_of(",")); // XXX: \s*,\s* ???
  for (const auto& w : widgets)
    if (!in_list<std::string>(w, {"splash", "playlist", "browser", "info", "help"}))
      throw std::invalid_argument(w + ": Invalid widget");
  return widgets;
}

std::vector<std::string> opt_parse_main_widgets(const std::string &s) {
  std::vector<std::string> widgets;
  boost::split(widgets, s, boost::is_any_of(",")); // XXX: \s*,\s* ???
  for (const auto& w : widgets)
    if (!in_list<std::string>(w, {"playinginfo", "tabbar", "mainwindow", "progressbar"}))
      throw std::invalid_argument(w + ": Invalid widget");
  return widgets;
}
/* ========================== */

PlaylistColumns opt_parse_playlist_format(const std::string &_xml) {
  std::vector<PlaylistColumnFormat> result;
  std::string xml = "<r>" + _xml + "</r>"; // Add fake root element
  XmlDoc doc = XmlDoc::readDoc(xml, NULL, NULL, XML_PARSE_COMPACT);
  XmlNode root = doc.getRootElement(); // Safe, we always have a root elem
  root = root.children();

  for (XmlNode node = root; node; node = node.next()) {
    //std::cout << "node type is: " << node.type() << std::endl;
    /*
    if (node->type == XML_TEXT_NODE)
      continue;
    */

    if (node.type() != XML_ELEMENT_NODE)
      continue;
      //throw std::invalid_argument("THIS IS NOT A ELEMENT NODE");

    if (node.children())
      throw std::invalid_argument("THIS NODE HAS CHILDREN");

    std::string column = node.name(); //XXX may be NULL?
    if (!in_list<std::string>(column, COLUMN_NAMES))
      throw std::invalid_argument(column + ": No such tag");

    PlaylistColumnFormat fmt;
    fmt.column = column;

    for (XmlAttribute attribute = node.attributes(); attribute; attribute = attribute.next()) {
      std::string name  = attribute.name();
      std::string value = attribute.value();

      if /**/ (name == "fg")      fmt.fg   = UI::Color::parse(value);
      else if (name == "bg")      fmt.bg   = UI::Color::parse(value);
      else if (name == "rel")     fmt.rel  = std::stoi(value);
      else if (name == "size")    fmt.size = std::stoi(value);
      else if (name == "justify") {
        if /**/ (value == "left")   fmt.justification = Left;
        else if (value == "right")  fmt.justification = Right;
        else throw std::invalid_argument("Invalid argument for justify");
      }
      else throw std::invalid_argument(name + ": Invalid attribute"); // XXX?
    }

    if (!fmt.size && !fmt.rel)
      throw std::invalid_argument("Must specify \"size\" or \"rel\"");
    if (fmt.size && fmt.rel)
      throw std::invalid_argument("Specify either \"size\" xor \"rel\""); // XXX?
  }

  return result;
}

//"<text fg='black'>&lt;&lt; </text><title bold='on' fg='yellow' /><text fg='black'> &gt;&gt;</text>";
PlayingInfoFormat opt_parse_playinginfo_format(const std::string &_xml) {
  PlayingInfoFormat result;
  std::string xml = "<r>" + _xml + "</r>"; // Add fake root element
  XmlDoc   doc = XmlDoc::readDoc(xml, NULL, NULL, XML_PARSE_COMPACT);
  XmlNode root = doc.getRootElement(); // Safe, always having root element

  for (XmlNode node = root.children(); node; node = node.next()) {
    PlayingInfoFormatFoo fmt;
    std::string tag = node.name();

    //std::cout << "node type is: " << node.type() << std::endl;
    /*
    if (node->type == XML_TEXT_NODE)
      continue;
    */

    if (node.type() != XML_ELEMENT_NODE)
      continue;
      //throw std::invalid_argument("THIS IS NOT A ELEMENT NODE");

    if (tag == "text") {
      fmt.text = node.text();
    }
    else {
      if (node.children())
        throw std::invalid_argument(tag + ": THIS NODE HAS CHILDREN");

      if (!in_list<std::string>(tag, COLUMN_NAMES))
        throw std::invalid_argument(tag + ": No such tag");
      fmt.tag = tag;
    }

    for (XmlAttribute attribute = node.attributes(); attribute; attribute = attribute.next()) {
      std::string name  = attribute.name();
      std::string value = attribute.value();

      if /**/ (name == "fg")   fmt.fg   = UI::Color::parse(value);
      else if (name == "bg")   fmt.bg   = UI::Color::parse(value);
      else                     fmt.attributes |= UI::Attribute::parse(name);
    }

    result.push_back(fmt);
  }

  return result;
}

/* Code generated by `./config.py cpp-decl` */
int                      Config :: threads;
int                      Config :: small_update_pages;
int                      Config :: playlist_load_newest;
bool                     Config :: prefetch;
bool                     Config :: use_cache;
bool                     Config :: tabbar_display;
bool                     Config :: playinginfo_display;
bool                     Config :: progressbar_display;
bool                     Config :: delete_after_extraction;
bool                     Config :: auto_extract_to_archive_dir;
char                     Config :: progressbar_rest_char;
char                     Config :: progressbar_progress_char;
std::string              Config :: log_file;
std::string              Config :: temp_dir;
std::string              Config :: cache_dir;
std::string              Config :: use_colors;
std::string              Config :: archive_dir;
std::string              Config :: audio_system;
std::string              Config :: download_dir;
std::string              Config :: database_file;
PlaylistColumns          Config :: browser_format;
PlaylistColumns          Config :: playlist_format;
PlaylistColumns          Config :: browser_format_256;
PlaylistColumns          Config :: playlist_format_256;
PlayingInfoFormat        Config :: playinginfo_format_top;
PlayingInfoFormat        Config :: playinginfo_format_bottom;
PlayingInfoFormat        Config :: playinginfo_format_top_256;
PlayingInfoFormat        Config :: playinginfo_format_bottom_256;
std::vector<std::string> Config :: main_widgets;
std::vector<std::string> Config :: tabs_widgets;

void Config :: init() {
  const std::string CONFIG_DIR = config_dir();
  const std::string HOME       = Filesystem::home();
  /* Code generated by `./config.py cpp-init` */
  threads = 20;
  small_update_pages = 5;
  playlist_load_newest = 1000;
  prefetch = true;
  use_cache = true;
  tabbar_display = true;
  playinginfo_display = true;
  progressbar_display = true;
  delete_after_extraction = true;
  auto_extract_to_archive_dir = true;
  progressbar_rest_char = '~';
  progressbar_progress_char = '~';
  log_file = "~/ektoplayer.log";
  temp_dir = "/tmp/.ektoplazm";
  cache_dir = "~/.cache/ektoplayer";
  use_colors = "auto";
  archive_dir = "~/.ektoplazm/archives";
  audio_system = "pulse,alsa,jack,oss";
  download_dir = "/tmp";
  database_file = "~/.ektoplazm/meta.db";
  browser_format = opt_parse_playlist_format(DEFAULT_PLAYLIST_FORMAT);
  playlist_format = opt_parse_playlist_format(DEFAULT_PLAYLIST_FORMAT);
  browser_format_256 = opt_parse_playlist_format(DEFAULT_PLAYLIST_FORMAT_256);
  playlist_format_256 = opt_parse_playlist_format(DEFAULT_PLAYLIST_FORMAT_256);
  playinginfo_format_top = opt_parse_playinginfo_format(DEFAULT_PLAYINGINFO_FORMAT_TOP);
  playinginfo_format_bottom = opt_parse_playinginfo_format(DEFAULT_PLAYINGINFO_FORMAT_BOTTOM);
  playinginfo_format_top_256 = opt_parse_playinginfo_format(DEFAULT_PLAYINGINFO_FORMAT_TOP_256);
  playinginfo_format_bottom_256 = opt_parse_playinginfo_format(DEFAULT_PLAYINGINFO_FORMAT_BOTTOM_256);
  main_widgets = opt_parse_main_widgets("playinginfo,tabbar,mainwindow,progressbar");
  tabs_widgets = opt_parse_tabs_widgets("splash,playlist,browser,info,help");
}

void Config :: set(const std::string &option, const std::string &value) {
  const std::string &o = option;
  if (0) {/* Code generated by `./config.py cpp-set` */}
  else if (o == "threads")     threads = validate_threads(value);
  else if (o == "log_file")    log_file = Filesystem::expand(value);
  else if (o == "prefetch")    prefetch = opt_parse_bool(value);
  else if (o == "temp_dir")    temp_dir = Filesystem::expand(value);
  else if (o == "cache_dir")   cache_dir = Filesystem::expand(value);
  else if (o == "use_cache")   use_cache = opt_parse_bool(value);
  else if (o == "use_colors")  use_colors = validate_use_colors(value);
  else if (o == "archive_dir") archive_dir = Filesystem::expand(value);
  else if (o == "audio_system") audio_system = value; // TODO: option parser
  else if (o == "download_dir") download_dir = Filesystem::expand(value);
  else if (o == "main.widgets") main_widgets = opt_parse_main_widgets(value);
  else if (o == "tabs.widgets") tabs_widgets = opt_parse_tabs_widgets(value);
  else if (o == "database_file") database_file = Filesystem::expand(value);
  else if (o == "browser.format") browser_format = opt_parse_playlist_format(value);
  else if (o == "tabbar.display") tabbar_display = opt_parse_bool(value);
  else if (o == "playlist.format") playlist_format = opt_parse_playlist_format(value);
  else if (o == "browser.format_256") browser_format_256 = opt_parse_playlist_format(value);
  else if (o == "small_update_pages") small_update_pages = opt_parse_int(value);
  else if (o == "playinginfo.display") playinginfo_display = opt_parse_bool(value);
  else if (o == "playlist.format_256") playlist_format_256 = opt_parse_playlist_format(value);
  else if (o == "progressbar.display") progressbar_display = opt_parse_bool(value);
  else if (o == "playlist_load_newest") playlist_load_newest = opt_parse_int(value);
  else if (o == "progressbar.rest_char") progressbar_rest_char = opt_parse_char(value);
  else if (o == "playinginfo.format_top") playinginfo_format_top = opt_parse_playinginfo_format(value);
  else if (o == "delete_after_extraction") delete_after_extraction = opt_parse_bool(value);
  else if (o == "playinginfo.format_bottom") playinginfo_format_bottom = opt_parse_playinginfo_format(value);
  else if (o == "progressbar.progress_char") progressbar_progress_char = opt_parse_char(value);
  else if (o == "playinginfo.format_top_256") playinginfo_format_top_256 = opt_parse_playinginfo_format(value);
  else if (o == "auto_extract_to_archive_dir") auto_extract_to_archive_dir = opt_parse_bool(value);
  else if (o == "playinginfo.format_bottom_256") playinginfo_format_bottom_256 = opt_parse_playinginfo_format(value);
  else throw std::invalid_argument("unknown option: " + option);
  // fail "Invalid value '#{value}' for '#{option}': #{$!}"
}

/*
void Config :: color(const std::vector<std::string> &vec) {
  int fg, bg, attr;
}
*/

void Config :: read(const std::string &file) {
  std::ifstream infile(file);
  std::string   line;
  unsigned int  no = 0;

  while (getline(infile, line)) {
    ++no;

    // line.chomp? next if line.empty? or start_with #
    std::vector<std::string> args = shellsplit(line);
    if (! args.size() || args[0][0] == '#')
      continue;

    std::string command = args[0];
#define on(STR) else if (command == STR)
    try {
      if (0) {}
      on("set")        { set(args[1], args[2]);     }
      /*
      on("bind")       { bindings.bind(...);        }
      on("unbind")     { bindings.unbind(...);      }
      on("unbind_all") { bindings.unbind_all(...);  }
      on("color")      { theme.color(...);          }
      on("color_256")  { theme.color_256(...);      }
      on("color_mono") { theme.color_mono(...);     }
      */
      else             { throw std::invalid_argument(command); }
    }
    catch (const std::exception &e) {
      char msg[4192];
      std::sprintf(msg, "%s:%u: %s: %s", file.c_str(), no, command.c_str(), e.what());
      throw std::invalid_argument(msg);
      //throw "#{file}:#{$.}: #{command}: #{$!}";
    }
  }
}

#if TEST_CONFIG
#include <cassert>
#include <iostream>
#include "colors.hpp"

int main() {
  //std::vector<PlaylistColumnFormat> r = parse_playlist_format(DEFAULT_PLAYLIST_FORMAT);
  try {
    assert(opt_parse_int("1") == 1);

    UI::Color::init();
    UI::Colors::init();
    UI::Attribute::init();
    Config::init();
    //c.set("foo", "bar");
  }
  catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }
}
#endif
