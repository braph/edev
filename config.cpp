#include "config.hpp"

#include "ektoplayer.hpp"
#include "ui/colors.hpp"
#include "lib/algorithm.hpp"
#include "lib/filesystem.hpp"
#include "lib/shellsplit.hpp"
#include "lib/stringpack.hpp"
#include "lib/raii/file.hpp"

#include <cstdio>
#include <cinttypes>

using namespace Views;
using pack = StringPack::AlnumNoCase;

// Parsing functions for primitives ===========================================

static int opt_parse_int(const std::string& s) {
  char *end;
  int i = std::strtoimax(s.c_str(), &end, 10);
  if (s.empty() || *end)
    throw std::invalid_argument(s + ": Not an integer");
  return i;
}

static bool opt_parse_bool(const std::string& s) {
  switch (pack::pack_runtime(s)) {
    case pack("true"):  return true;
    case pack("false"): return false;
  }
  throw std::invalid_argument(s + ": Expected `true` or `false`");
}

static wchar_t opt_parse_char(const std::string& s) {
  size_t len;
  wchar_t* ws = toWideString(s, &len);
  if (len != 1)
    throw std::invalid_argument(s + ": Expected a single character");
  return *ws;
}

// Parsing functions for `special` primitives =================================

static int opt_parse_use_colors(const std::string& s) {
  switch (pack::pack_runtime(s)) {
    case pack("auto"):  return -1;
    case pack("mono"):  return 0;
    case pack("8"):     return 8;
    case pack("256"):   return 256;
  }
  throw std::invalid_argument(s + ": Expected auto|mono|8|256");
}

static short opt_parse_color(const std::string& s) {
  auto color = UI::Color::parse(s);
  if (! color.ok)
    throw std::invalid_argument(s + ": Invalid color");
  return color;
}

static unsigned int opt_parse_attribute(const std::string& s) {
  auto attr = UI::Attribute::parse(s);
  if (! attr.ok)
    throw std::invalid_argument(s + ": Invalid attribute");
  return attr;
}

static Database::ColumnID opt_parse_column(const std::string& s) {
  Database::ColumnID column = Database::columnIDFromStr(s);
  if (column == Database::COLUMN_NONE)
    throw std::invalid_argument(s + ": No such column");
  return column;
}
      
// Parsing functions for complex objects ======================================

static decltype(Config::tabs_widgets) opt_parse_tabs_widgets(const std::string& s) {
  decltype(Config::tabs_widgets) widgets = {};
  size_t idx = 0;
  const char *cs = s.c_str();

  while (cs += std::strspn(cs, ", \t"), *cs) {
    size_t len = std::strcspn(cs, ", \t");
    std::string w(cs, len);
    cs += len;

    switch (pack::pack_runtime(w)) {
      case pack("splash"):    widgets[idx] = TabWidgets::SPLASH;   break;
      case pack("playlist"):  widgets[idx] = TabWidgets::PLAYLIST; break;
      case pack("browser"):   widgets[idx] = TabWidgets::BROWSER;  break;
      case pack("info"):      widgets[idx] = TabWidgets::INFO;     break;
      case pack("help"):      widgets[idx] = TabWidgets::HELP;     break;
      default:                throw std::invalid_argument(w + ": Invalid widget");
    }
    ++idx; // XXX We don't care about bounds checking
  }
  return widgets;
}

static decltype(Config::main_widgets) opt_parse_main_widgets(const std::string &s) {
  decltype(Config::main_widgets) widgets = {};
  size_t idx = 0;
  const char *cs = s.c_str();

  while (cs += std::strspn(cs, ", \t"), *cs) {
    size_t len = std::strcspn(cs, ", \t");
    std::string w(cs, len);
    cs += len;

    using pack = StringPack::AlphaNoCase;
    switch (pack::pack_runtime(w)) {
      case pack("infoline"):    widgets[idx] = MainWidgets::INFOLINE;    break;
      case pack("tabbar"):      widgets[idx] = MainWidgets::TABBAR;      break;
      case pack("readline"):    widgets[idx] = MainWidgets::READLINE;    break;
      case pack("windows"):     widgets[idx] = MainWidgets::WINDOWS;     break;
      case pack("progressbar"): widgets[idx] = MainWidgets::PROGRESSBAR; break;
      default:                  throw std::invalid_argument(w + ": Invalid widget");
    }
    ++idx; // XXX We don't care about bounds checking
  }
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

      column = opt_parse_column(text);

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
      switch (pack::pack_runtime(attr.name)) {
        case pack("fg"):    fmt.fg = opt_parse_color(attr.value);  break;
        case pack("bg"):    fmt.bg = opt_parse_color(attr.value);  break;
        case pack("right"): fmt.justify = PlaylistColumnFormat::Justify::Right;  break;
        case pack("left"):  fmt.justify = PlaylistColumnFormat::Justify::Left;   break;
        case pack("size"):
          fmt.size = std::atoi(attr.value.c_str());
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
    InfoLineFormatString fmt;

    if (formatParser.column)
      fmt.tag  = formatParser.column;
    else
      fmt.text = formatParser.text;

    auto attr = formatParser.attributes();
    while (attr.next()) {
      switch (pack::pack_runtime(attr.name)) {
        case pack("fg"): fmt.fg = opt_parse_color(attr.value); break;
        case pack("bg"): fmt.bg = opt_parse_color(attr.value); break;
        default:         fmt.attributes |= opt_parse_attribute(attr.name);
      }
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

static inline void checkArgCount(const std::vector<std::string>& args, size_t min, size_t max) {
  if (args.size() < min)
    throw std::invalid_argument("Missing arguments");
  if (args.size() > max)
    throw std::invalid_argument("Too many arguments");
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
  auto it = make_iterator_pair(args);

  Theme::ThemeID theme;
  using pack = ;
  switch (pack::pack_runtime(it.next())) {
    case pack("color_mono"): theme = Theme::MONO;   break;
    case pack("color"):      theme = Theme::EIGHT;  break;
    case pack("color_256"):  theme = Theme::FULL;   break;
    default:                 throw std::invalid_argument(args[0] + ": Invalid theme");
  }

  const std::string& themeElement = it.next();
  short fg = parse(it.next());
  short bg = parse(it.next());
  unsigned int attrs = 0;
  while (it)
    attrs |= parse(it.next());
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

static bool getnextline(FILE* fh, std::string& line) {
  line.clear();
  int c = std::fgetc(fh);
  if (c == EOF)  return false;
  if (c == '\n') return true;
  do {
    line.push_back(c);
    c = std::fgetc(fh);
  }
  while (c != EOF && c != '\n');
  return true;
}

void Config :: read(const std::string &file) {
  RAII::FILE fh( std::fopen(file.c_str(), "r") );
  if (! fh)
    throw std::runtime_error(std::strerror(errno));

  std::string line;
  std::vector<std::string> args;
  unsigned int no = 0;

  try {
    while (getnextline(fh, line)) {
      ++no;
      ShellSplit::split(line, args);
      if (! args.size() || args[0][0] == '#')
        continue;

      switch (pack::pack_runtime(args[0])) {
        case pack("set"):        Config::set(args);        break;
        case pack("color"):      Config::color(args);      break;
        case pack("color_256"):  Config::color(args);      break;
        case pack("color_mono"): Config::color(args);      break;
        case pack("bind"):       Config::bind(args);       break;
        case pack("unbind"):     Config::unbind(args);     break;
        case pack("unbind_all"): Config::unbind_all(args); break;
        default: throw std::invalid_argument("unknown command");
      }
    }
  }
  catch (const std::exception &e) {
    char _[1024];
    std::snprintf(_, sizeof(_), "%s:%u: %s: %s", file.c_str(), no, args[0].c_str(), e.what());
    throw std::invalid_argument(_);
  }

  if (std::ferror(fh))
    throw std::runtime_error(std::strerror(EIO));
}

#ifdef TEST_CONFIG
#include "lib/test.hpp"
#include "ui/colors.hpp"
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
  { auto c = opt_parse_tabs_widgets("  help , INFO ");
    assert(c[0] == TabWidgets::HELP);
    assert(c[1] == TabWidgets::INFO);
    assert(c[2] == TabWidgets::NONE);
  }

  { auto c = opt_parse_main_widgets("  infoline , PROGRESSBAR ");
    assert(c[0] == MainWidgets::INFOLINE);
    assert(c[1] == MainWidgets::PROGRESSBAR);
    assert(c[2] == MainWidgets::NONE);
  }

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
