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
  "'<< '{fg=black} title{fg=yellow,bold} ' >>'{fg=black}";

static const char* const DEFAULT_PLAYINGINFO_FORMAT_BOTTOM = 
  "artist{fg=blue,bold} ' - ' album{fg=red,bold} ' (' year{fg=cyan} ')'";

static const char* const DEFAULT_PLAYINGINFO_FORMAT_TOP_256 =
  "'<< '{fg=236} title{fg=178,bold} ' >>'{fg=236}";

static const char* const DEFAULT_PLAYINGINFO_FORMAT_BOTTOM_256 = 
  "artist{fg=24,bold} ' - ' album{fg=160,bold} ' (' year{fg=37} ')'";

static const char* const DEFAULT_PLAYLIST_COLUMNS =
  "number{fg=magenta size=3} artist{fg=blue size=25%} album{fg=red size=30%}"
  "title {fg=yellow size=33%} styles{fg=cyan size=20%} bpm{fg=green size=3 right}";

static const char* const DEFAULT_PLAYLIST_COLUMNS_256 =
  "number{fg=97 size=3} artist{fg=24 size=25%} album{fg=160 size=30%}"
  "title {fg=178 size=33%} styles{fg=37 size=20%} bpm{fg=28 size=3 right}";

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

// OLD STUFF
static inline const char* skipWhitespace(const char *s) {
  while (*s && (*s == ' ' || *s == '\t')) { ++s; }
  return s;
}
#if 0
static inline const char* skipWhitespace(const char *s) {
  return s + std::strspn(s, " \t");
}
#endif

struct AttributeParser {
  const char* s;
  std::string opt, value;
  AttributeParser(const std::string& s) : s(s.c_str()) {}

  bool next() {
    opt.clear();
    value.clear();
    std::string* current = &opt;
    s = skipWhitespace(s);
    for (; *s; ++s) {
      if (*s == ',' || isspace(*s))   { break;                  }
      else if (*s == '=')             { current = &value;       }
      else                            { current->push_back(*s); }
    }

    return !opt.empty();
  }
};

struct FormatParser {
  const char* s;
  Database::ColumnID column;
  std::string text;
  std::string _attributes;
  FormatParser(const std::string& str) : s(str.c_str()) {}

  bool next() {
    column = Database::COLUMN_NONE;
    text.clear();
    _attributes.clear();
    s = skipWhitespace(s);

    if (*s == '\'' || *s == '"') { // Its a string literal
      char quote = *s++;
      bool escaped = false;
      for (; *s; ++s) {
        /**/ if (escaped)     { text.push_back(*s); escaped = false; }
        else if (*s == '\\')  { escaped = true;                      }
        else if (*s == quote) { break;                               }
        else                  { text.push_back(*s);                  }
      }
    }
    else {
      for (; *s && std::isalnum(*s); ++s)
        text.push_back(*s);

      if (text.empty())
        return false;

      column = Database::columnIDFromStr(text);
      if (column == Database::COLUMN_NONE)
        throw std::invalid_argument(text + ": No such tag");
    }

    s = skipWhitespace(s);
    if (*s == '{') { // Having attributes
      ++s;
      for (; *s && *s != '}'; ++s)
        _attributes.push_back(*s);
      ++s;
    }

    return true;
  }

  AttributeParser attributes() {
    return AttributeParser(_attributes);
  }
};

static PlaylistColumns opt_parse_playlist_columns(const std::string &s) { // MOCKUP
  PlaylistColumns result;
  FormatParser formatParser(s);
  while (formatParser.next()) {
    PlaylistColumnFormat fmt;

    if (formatParser.column == Database::COLUMN_NONE)
      throw std::invalid_argument("Missing column name");

    fmt.tag = formatParser.column;

    auto attr = formatParser.attributes();
    while (attr.next()) {
      /**/ if (attr.opt == "fg")     fmt.fg = UI::Color::parse(attr.value);
      else if (attr.opt == "bg")     fmt.bg = UI::Color::parse(attr.value);
      else if (attr.opt == "right")  fmt.justify = PlaylistColumnFormat::Right;
      else if (attr.opt == "left")   fmt.justify = PlaylistColumnFormat::Left;
      else if (attr.opt == "size") {
        fmt.size = std::stoi(attr.value);
        fmt.relative = (attr.value.back() == '%');
      }
    }

    if (! fmt.size)
      throw std::invalid_argument(formatParser.text + ": Missing column size");

    result.push_back(fmt);
  }

  return result;
}

//"<text fg='black'>&lt;&lt; </text><title bold='on' fg='yellow' /><text fg='black'> &gt;&gt;</text>";
static PlayingInfoFormat opt_parse_playinginfo_format(const std::string& s) {
  PlayingInfoFormat result;

  FormatParser formatParser(s);
  while (formatParser.next()) {
    PlayingInfoFormatFoo fmt;

    if (formatParser.column)
      fmt.tag  = formatParser.column;
    else
      fmt.text = formatParser.text;

    auto attr = formatParser.attributes();
    while (attr.next()) {
      /**/ if (attr.opt == "fg")   fmt.fg = UI::Color::parse(attr.value);
      else if (attr.opt == "bg")   fmt.bg = UI::Color::parse(attr.value);
      else fmt.attributes |= UI::Attribute::parse(attr.opt);
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

#if TEST_CONFIG
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
  c = opt_parse_playlist_columns("number{fg=blue bg=magenta size=3} artist{size=10%}");
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
