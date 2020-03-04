#include "config.hpp"

#include "ektoplayer.hpp"
#include "filesystem.hpp"
#include "shellsplit.hpp"
#include "common.hpp"
#include "colors.hpp"
#include "xml.hpp"

#include <boost/algorithm/string.hpp>

#include <cinttypes>
#include <fstream>

using namespace Views;

static const char* const DEFAULT_PLAYINGINFO_FORMAT_TOP =
  "<text fg='black'>&lt;&lt; </text><title bold='on' fg='yellow' /><text fg='black'> &gt;&gt;</text>";

static const char* const DEFAULT_PLAYINGINFO_FORMAT_BOTTOM = 
  "<artist bold='on' fg='blue' /><text> - </text><album bold='on' fg='red' /><text> (</text><year fg='cyan' /><text>)</text>";

static const char* const DEFAULT_PLAYINGINFO_FORMAT_TOP_256 =
  "<text fg='236'>&lt;&lt; </text><title bold='on' fg='178' /><text fg='236'> &gt;&gt;</text>";

static const char* const DEFAULT_PLAYINGINFO_FORMAT_BOTTOM_256 = 
 "<artist bold='on' fg='24' /><text> - </text><album bold='on' fg='160' /><text> (</text><year fg='37' /><text>)</text>";

#if I_FOUND_A_BETTER_SOLUTION_THAN_XML
static const char* const DEFAULT_PLAYINGINFO_FORMAT_TOP =
  "{fg=black}<< {fg=yellow,bold}${title} {fg=black}>>"

static const char* const DEFAULT_PLAYINGINFO_FORMAT_BOTTOM = 
  "{fg=blue,bold}${artist} <text> - </text><album bold='on' fg='red' /><text> (</text><year fg='cyan' /><text>)</text>";
  "<artist bold='on' fg='blue' /><text> - </text><album bold='on' fg='red' /><text> (</text><year fg='cyan' /><text>)</text>";
  "{artist fg=blue bold} {text - } ";

static const char* const DEFAULT_PLAYINGINFO_FORMAT_TOP_256 =
  "<text fg='236'>&lt;&lt; </text><title bold='on' fg='178' /><text fg='236'> &gt;&gt;</text>";

static const char* const DEFAULT_PLAYINGINFO_FORMAT_BOTTOM_256 = 
 "<artist bold='on' fg='24' /><text> - </text><album bold='on' fg='160' /><text> (</text><year fg='37' /><text>)</text>";
#endif

static const char* const DEFAULT_PLAYLIST_COLUMNS =
  "number 3 fg=magenta  | artist 25% fg=blue | album 30% fg=red |"
  "title  33% fg=yellow | styles 20% fg=cyan | bpm   3 fg=green right";

static const char* const DEFAULT_PLAYLIST_COLUMNS_256 =
  "number 3 fg=97    | artist 25% fg=24 | album 30% fg=160 |"
  "title  33% fg=178 | styles 20% fg=37 | bpm   3 fg=28 right";

/* === Parsing for primitives === */
static int opt_parse_int(const std::string &s) {
  char *end;
  int i = std::strtoimax(s.c_str(), &end, 10);
  if (!s.size() || end != s.c_str() + s.size())
    throw std::invalid_argument("not an integer");
  return i;
}

static bool opt_parse_bool(const std::string &s) {
  if (s == "true")        return true;
  else if (s == "false")  return false;
  else throw std::invalid_argument("expected `true` or `false`");
}

static char opt_parse_char(const std::string &s) {
  if (s.size() != 1)
    throw std::invalid_argument("expected a single character");
  return s[0];
}
/* === End of primitives === */

/* === Option parsing functions === */
static int opt_parse_use_colors(const std::string &s) {
  /**/ if (s == "auto")   return -1;
  else if (s == "mono")   return 0;
  else if (s == "8")      return 8;
  else if (s == "256")    return 256;
  throw std::invalid_argument("expected auto|mono|8|256");
}

static int opt_parse_threads(const std::string &s) {
  int i = opt_parse_int(s);
  if (i < 1)
    throw std::invalid_argument("integer must be > 1");
  return i;
}

static std::vector<std::string> opt_parse_tabs_widgets(const std::string &s) {
  std::vector<std::string> widgets;
  boost::split(widgets, s, boost::is_any_of(", \t"), boost::token_compress_on);
  for (const auto& w : widgets)
    if (!in_list<std::string>(w, {"splash", "playlist", "browser", "info", "help"}))
      throw std::invalid_argument(w + ": Invalid widget");
  return widgets;
}

static std::vector<std::string> opt_parse_main_widgets(const std::string &s) {
  std::vector<std::string> widgets;
  boost::split(widgets, s, boost::is_any_of(", \t"), boost::token_compress_on);
  for (const auto& w : widgets)
    if (!in_list<std::string>(w, {"playinginfo", "tabbar", "windows", "progressbar"}))
      throw std::invalid_argument(w + ": Invalid widget");
  return widgets;
}

static PlaylistColumns opt_parse_playlist_columns(const std::string &s) {
  std::vector<std::string> opts;
  std::vector<std::string> columns;
  boost::split(columns, s, boost::is_any_of("|"));

  PlaylistColumns result;
  for (auto &column : columns) {
    PlaylistColumnFormat fmt;

    boost::split(opts, column, boost::is_any_of(" \t"), boost::token_compress_on);
    for (auto &opt : opts) {
      if (opt.empty())
        continue;
      else if (0 == opt.find("fg="))
        fmt.fg = UI::Color::parse(opt.substr(3));
      else if (0 == opt.find("bg="))
        fmt.bg = UI::Color::parse(opt.substr(3));
      else if (opt == "right")
        fmt.justify = PlaylistColumnFormat::Right;
      else if (opt == "left")
        fmt.justify = PlaylistColumnFormat::Left;
      else if (std::isdigit(opt[0])) {
        fmt.size = std::stoi(opt);
        fmt.relative = (opt.back() == '%');
      }
      else {
        fmt.tag = (Database::ColumnID) Database::columnIDFromStr(opt);
        if (fmt.tag == Database::COLUMN_NONE)
          throw std::invalid_argument(opt + ": No such tag");
      }
    }

    if (! fmt.size)
      throw std::invalid_argument(column + ": Missing column size");
    if (fmt.tag == Database::COLUMN_NONE)
      throw std::invalid_argument(column + ": Missing tag name");

    result.push_back(fmt);
  }

  return result;
}

//"<text fg='black'>&lt;&lt; </text><title bold='on' fg='yellow' /><text fg='black'> &gt;&gt;</text>";
static PlayingInfoFormat opt_parse_playinginfo_format(const std::string &_xml) {
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

      fmt.tag = (Database::ColumnID) Database::columnIDFromStr(tag);
      if (fmt.tag == Database::COLUMN_NONE)
        throw std::invalid_argument(tag + ": No such tag");
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
/* ========================== */

#include "config/config.members.declare.cpp"

void Config :: init() {
  const std::string CONFIG_DIR = Ektoplayer::config_dir();
  const std::string HOME       = Filesystem::home();
#include "config/config.members.initialize.cpp"
}

void Config :: set(const std::string &option, const std::string &value) {
  try { if (0) {}
#include "config/config.members.set.cpp"
    else throw false;
  } catch (const std::exception &e) {
    throw std::invalid_argument(option + ": " + e.what());
  } catch (bool e) {
    throw std::invalid_argument("unknown option: " + option);
  }
}

void Config :: read(const std::string &file) {
  std::ifstream infile;
  infile.exceptions(std::ifstream::badbit|std::ifstream::failbit);
  infile.open(file);
  std::string   line;
  std::string   command;
  unsigned int  no = 0;

  try {
    while (getline(infile, line)) {
      ++no;

      std::vector<std::string> args = shellsplit(line);
      if (! args.size() || args[0][0] == '#')
        continue;

      command = args[0];
#define on(STR) else if (command == STR)
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
      else             { throw std::invalid_argument("unknown command"); }
    }
  }
  catch (const std::ifstream::failure &e) {
    if (! infile.eof())
      throw;
  }
  catch (const std::exception &e) {
    char _[1024];
    snprintf(_, sizeof(_), "%s:%u: %s: %s", file.c_str(), no, command.c_str(), e.what());
    throw std::invalid_argument(_);
  }
}

#ifdef TEST_CONFIG
#include "test.hpp"
#include "colors.hpp"
int main() {
  TEST_BEGIN();

  Config::init();

  // === Primitives ===
  assert(opt_parse_int("1")       == 1);
  except(opt_parse_int(""));
  except(opt_parse_int("-"));
  except(opt_parse_int("1a"));
  except(opt_parse_int("a1"));

  assert(opt_parse_bool("true")   == true);
  assert(opt_parse_bool("false")  == false);
  except(opt_parse_bool(""));
  except(opt_parse_bool("-"));

  assert(opt_parse_char("c")      == 'c');
  except(opt_parse_char(""));
  except(opt_parse_char("12"));

  // === Objects ===
  PlaylistColumns c;
  c = opt_parse_playlist_columns("number 3 fg=blue bg=magenta | artist 10%");
  assert(c.size()       == 2);
  assert(c[0].size      == 3);
  assert(c[0].relative  == false);
  assert(c[0].fg        == COLOR_BLUE);
  assert(c[0].bg        == COLOR_MAGENTA);
  assert(c[1].size      == 10);
  assert(c[1].relative  == true);
  assert(c[1].fg        == -1);
  assert(c[1].bg        == -1);

  TEST_END();
}
#endif
