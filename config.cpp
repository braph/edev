#include "config.hpp"

#include "ektoplayer.hpp"
#include "filesystem.hpp"
#include "shellsplit.hpp"
#include "common.hpp"
#include "colors.hpp"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp> // is_any_of()

#include <fstream>
#include <cinttypes>

using namespace Views;

/* ============================================================================
 * Parsing functions for primitives
 * ==========================================================================*/

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

static wchar_t opt_parse_char(const std::string &s) {
  size_t len;
  wchar_t* ws = toWideString(s, &len);
  if (len != 1)
    throw std::invalid_argument("expected a single character");
  return *ws;
}

/* ============================================================================
 * Option parsing functions for primitives
 * ==========================================================================*/

static int opt_parse_use_colors(const std::string &s) {
  /**/ if (s == "auto")   return -1;
  else if (s == "mono")   return 0;
  else if (s == "8")      return 8;
  else if (s == "256")    return 256;
  throw std::invalid_argument("expected auto|mono|8|256");
}

static std::vector<std::string> opt_parse_tabs_widgets(const std::string &s) {
  std::vector<std::string> widgets;
  boost::split(widgets, s, boost::is_any_of(", \t"), boost::token_compress_on);
  for (const auto& w : widgets)
    if (!in_list<std::string>(w, {"splash","playlist","browser","info","help"}))
      throw std::invalid_argument(w + ": Invalid widget");
  return widgets;
}

static std::vector<std::string> opt_parse_main_widgets(const std::string &s) {
  std::vector<std::string> widgets;
  boost::split(widgets, s, boost::is_any_of(", \t"), boost::token_compress_on);
  for (const auto& w : widgets)
    if (!in_list<std::string>(w, {"infoline","tabbar","readline","windows","progressbar"}))
      throw std::invalid_argument(w + ": Invalid widget");
  return widgets;
}

static inline const char* skipWhitespace(const char *s) {
  while (*s && (*s == ' ' || *s == '\t')) { ++s; }
  return s;
}

struct AttributeParser {
  const char* s;
  std::string name, value;
  AttributeParser(const std::string& s) : s(s.c_str()) {}

  bool next() {
    name.clear();
    value.clear();
    std::string* current = &name;
    s = skipWhitespace(s);
    for (; *s; ++s) {
      if (*s == ',' || isspace(*s)) { break;                  }
      else if (*s == '=')           { current = &value;       }
      else                          { current->push_back(*s); }
    }

    return !name.empty();
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
      char quote = *s;
      bool escaped = false;
      for (++s; *s; ++s) {
        /**/ if (escaped)     { text.push_back(*s); escaped = false; }
        else if (*s == '\\')  { escaped = true;                      }
        else if (*s == quote) { ++s; break;                          }
        else                  { text.push_back(*s);                  }
      }
    }
    else {
      for (; std::isalnum(*s); ++s)
        text.push_back(*s);

      if (text.empty())
        return false;

      column = Database::columnIDFromStr(text);
      if (column == Database::COLUMN_NONE)
        throw std::invalid_argument(text + ": No such tag");

      text.clear();
    }

    s = skipWhitespace(s);
    if (*s == '{') { // Having attributes
      while (*++s)
        if (*s == '}')
           { ++s; break; }
        else
          _attributes.push_back(*s);
    }

    return true;
  }

  AttributeParser attributes() {
    return AttributeParser(_attributes);
  }
};

static PlaylistColumns opt_parse_playlist_columns(const std::string &s) {
  PlaylistColumns result;
  FormatParser formatParser(s);
  while (formatParser.next()) {
    PlaylistColumnFormat fmt;

    if (formatParser.column == Database::COLUMN_NONE)
      throw std::invalid_argument("Missing column name");

    fmt.tag = formatParser.column;

    auto attr = formatParser.attributes();
    while (attr.next()) {
      /**/ if (attr.name == "fg")     fmt.fg = UI::Color::parse(attr.value);
      else if (attr.name == "bg")     fmt.bg = UI::Color::parse(attr.value);
      else if (attr.name == "right")  fmt.justify = PlaylistColumnFormat::Right;
      else if (attr.name == "left")   fmt.justify = PlaylistColumnFormat::Left;
      else if (attr.name == "size") {
        fmt.size = std::stoi(attr.value);
        fmt.relative = (attr.value.back() == '%');
      }
    }

    if (! fmt.size)
      throw std::invalid_argument(formatParser.text + ": Missing column size");

    result.push_back(std::move(fmt));
  }

  return result;
}

//"<text fg='black'>&lt;&lt; </text><title bold='on' fg='yellow' /><text fg='black'> &gt;&gt;</text>";
static InfoLineFormat opt_parse_infoline_format(const std::string& s) {
  InfoLineFormat result;

  FormatParser formatParser(s);
  while (formatParser.next()) {
    InfoLineFormatFoo fmt;

    if (formatParser.column)
      fmt.tag  = formatParser.column;
    else
      fmt.text = formatParser.text;

    auto attr = formatParser.attributes();
    while (attr.next()) {
      /**/ if (attr.name == "fg")  fmt.fg = UI::Color::parse(attr.value);
      else if (attr.name == "bg")  fmt.bg = UI::Color::parse(attr.value);
      else fmt.attributes |= UI::Attribute::parse(attr.name);
    }

    result.push_back(std::move(fmt));
  }

  return result;
}

/* ============================================================================
 * End of option parsing functions
 * ==========================================================================*/

#include "config/options.define.cpp"

void Config :: init() {
#include "config/options.initialize.cpp"
}

#define TOO_MANY_ARGUMENTS "Too many arguments"
#define MISSING_ARGUMENTS  "Missing arguments"

static inline void checkArgCount(const std::vector<std::string>& args, size_t min, size_t max) {
  if (args.size() < min)
    throw std::invalid_argument(MISSING_ARGUMENTS);
  if (args.size() > max)
    throw std::invalid_argument(TOO_MANY_ARGUMENTS);
}

void Config :: set(const std::vector<std::string>& args) {
  checkArgCount(args, 3, 3);
  const std::string& option = args[1];
  const std::string& value  = args[2];

  try {
    if (0) {}
#include "config/options.set.cpp"
    else throw std::invalid_argument("unknown option");
  } catch (const std::exception &e) {
    throw std::invalid_argument(option + ": " + e.what());
  }
}

void Config :: color(const std::vector<std::string>& args) {
  checkArgCount(args, 3, INT_MAX);

#if /*TODO*/ 0
  auto it = args.cbegin();
  auto end = args.cend();

  Theme::ThemeID theme;
  if (args[0] == "color_mono")
    theme = Theme::MONO;
  else if (args[0] == "color")
    theme = Theme::EIGHT;
  else if (args[0] == "color_256")
    theme = Theme::FULL;

  unsigned int attrs;
  short fg;
  short bg;

  const std::string& themeElement = args[1];
  auto it = std::next(args.cbegin(), 2);

  for (
#endif
}

void Config :: bind(const std::vector<std::string>& args) {
  abort();
}

void Config :: unbind(const std::vector<std::string>& args) {
  abort();
}

void Config :: unbind_all(const std::vector<std::string>& args) {
  abort();
}

void Config :: read(const std::string &file) {
  std::ifstream infile;
  infile.exceptions(std::ifstream::badbit|std::ifstream::failbit);
  infile.open(file);
  std::string line;
  std::vector<std::string> args;
  unsigned int no = 0;

  try {
    while (getline(infile, line)) {
      ++no;
      ShellSplit::split(line, args);
      if (! args.size() || args[0][0] == '#')
        continue;
      else if (args[0] == "set")        { Config::set(args);        }
      else if (args[0] == "color")      { Config::color(args);      }
      else if (args[0] == "color_256")  { Config::color(args);      }
      else if (args[0] == "color_mono") { Config::color(args);      }
      else if (args[0] == "bind")       { Config::bind(args);       }
      else if (args[0] == "unbind")     { Config::unbind(args);     }
      else if (args[0] == "unbind_all") { Config::unbind_all(args); }
      else throw std::invalid_argument("unknown command");
    }
  }
  catch (const std::ifstream::failure&) {
    if (! infile.eof())
      throw;
  }
  catch (const std::exception &e) {
    char _[1024];
    snprintf(_, sizeof(_), "%s:%u: %s: %s", file.c_str(), no, args[0].c_str(), e.what());
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
  assert(opt_parse_int("1") == 1);
  except(opt_parse_int(""));
  except(opt_parse_int("-"));
  except(opt_parse_int("1a"));
  except(opt_parse_int("a1"));

  assert(opt_parse_bool("true") == true);
  assert(opt_parse_bool("false") == false);
  except(opt_parse_bool(""));
  except(opt_parse_bool("-"));

  assert(opt_parse_char("c") == 'c');
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
  InfoLineFormat fmt;
  fmt = opt_parse_infoline_format("'>>'{fg=blue bg=magenta bold}");
  assert(fmt.size()         == 1);
  assert(fmt[0].text        == ">>");
  assert(fmt[0].fg          == COLOR_BLUE);
  assert(fmt[0].bg          == COLOR_MAGENTA);
  assert(fmt[0].attributes  == A_BOLD);

  TEST_END();
}
#endif
