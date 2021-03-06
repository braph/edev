#include "config.hpp"

#include "ektoplayer.hpp"
#include "ui/colors.hpp"
#include "bindings.hpp"
#include <lib/algorithm.hpp>
#include <lib/filesystem.hpp>
#include <lib/shellsplit.hpp>
#include <lib/stringpack.hpp>
#include <lib/cfile.hpp>
#include <lib/hash.hpp>

#include <vector>
#include <cstdio>
#include <cinttypes>
#include <algorithm>

using namespace Views;

// Parsing functions for primitives ===========================================

static int parse_int(ConstChars s) {
  char *end;
  int i = std::strtoimax(s, &end, 10);
  if (!*s || *end)
    throw ConfigError(s, "Not an integer");
  return i;
}

static float parse_float(ConstChars s) {
  char *end;
  float f = std::strtof(s, &end);
  if (!*s || *end)
    throw ConfigError(s, "Not a float");
  return f;
}

static bool parse_bool(ConstChars s) {
  using pack = StringPack::AlnumNoCase;
  switch (pack(s)) {
    case pack("true"):  return true;
    case pack("false"): return false;
  }
  throw ConfigError(s, "Expected `true` or `false`");
}

static wchar_t parse_char(ConstChars s) {
  wchar_t wide[2];
  if (1 != std::mbstowcs(wide, s, 2))
    throw ConfigError(s, "Expected a single character");
  return wide[0];
}

// Parsing functions for `special` primitives =================================

static int parse_use_colors(ConstChars s) {
  using pack = StringPack::AlnumNoCase;
  switch (pack(s)) {
    case pack("auto"):  return -1;
    case pack("mono"):  return 0;
    case pack("8"):     return 8;
    case pack("256"):   return 256;
  }
  throw ConfigError(s, "Expected auto|mono|8|256");
}

static short parse_color(ConstChars s) {
  short color = UI::Color::parse(s.s);
  if (color != UI::Color::Invalid)
    return color;
  throw ConfigError(s, "Invalid color");
}

static attr_t parse_attribute(ConstChars s) {
  attr_t attr = UI::Attribute::parse(s.s);
  if (attr != UI::Attribute::Invalid)
    return attr;
  throw ConfigError(s, "Invalid attribute");
}

static Database::ColumnID parse_column(ConstChars s) {
  Database::ColumnID column = Database::columnIDFromStr(s);
  if (column != Database::COLUMN_NONE)
    return column;
  throw ConfigError(s, "No such column");
}

// Parsing functions for complex objects ======================================

static decltype(Config::tabs_widgets) parse_tabs_widgets(ConstChars s) {
  decltype(Config::tabs_widgets) widgets = {};
  size_t idx = 0;
  const char *cs = s;

  while (cs += std::strspn(cs, ", \t"), *cs) {
    size_t len = std::strcspn(cs, ", \t");
    std::string w(cs, len);
    cs += len;

    int val = 0;
    using pack = StringPack::AlnumNoCase;
    switch (pack(w)) {
      default:                throw ConfigError(w, "Invalid widget");
      case pack("help"):      ++val; // fall-through
      case pack("info"):      ++val; // fall-through
      case pack("browser"):   ++val; // fall-through
      case pack("playlist"):  ++val; // fall-through
      case pack("splash"):    ++val; // fall-through
                              widgets[idx++] = static_cast<TabWidgets>(val);
    }
  }
  return widgets;
}

static decltype(Config::main_widgets) parse_main_widgets(ConstChars s) {
  decltype(Config::main_widgets) widgets = {};
  size_t idx = 0;
  const char *cs = s;

  while (cs += std::strspn(cs, ", \t"), *cs) {
    size_t len = std::strcspn(cs, ", \t");
    std::string w(cs, len);
    cs += len;

    int val = 0;
    using pack = StringPack::AlphaNoCase;
    switch (pack(w)) {
      default:                  throw ConfigError(w, "Invalid widget");
      case pack("progressbar"): ++val; // fall-through
      case pack("windows"):     ++val; // fall-through
      case pack("readline"):    ++val; // fall-through
      case pack("tabbar"):      ++val; // fall-through
      case pack("infoline"):    ++val; // fall-through
                                widgets[idx++] = static_cast<MainWidgets>(val);
    }
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
  AttributeParser(ConstChars s) : s(s) {}

  bool next() {
    name.clear();
    value.clear();
    std::string* current = &name;
    s = skipWhitespace(s);
    for (; *s; ++s) {
      if (*s == ',' || *s == ' ' || *s == '\t' || *s == '\n') { break; }
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
  FormatParser(ConstChars str) : s(str) {}

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

      column = parse_column(text);

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

static PlaylistColumns parse_playlist_columns(ConstChars s) {
  PlaylistColumns result;
  FormatParser formatParser(s);
  while (formatParser.next()) {
    PlaylistColumnFormat fmt;

    if (formatParser.column == Database::COLUMN_NONE)
      throw ConfigError("Missing column name");

    fmt.tag = formatParser.column;

    auto attr = formatParser.attributes();
    while (attr.next()) {
      using pack = StringPack::AlnumNoCase;
      switch (pack(attr.name)) {
        case pack("fg"):    fmt.fg = parse_color(attr.value);  break;
        case pack("bg"):    fmt.bg = parse_color(attr.value);  break;
        case pack("right"): fmt.justify = PlaylistColumnFormat::Justify::Right;  break;
        case pack("left"):  fmt.justify = PlaylistColumnFormat::Justify::Left;   break;
        case pack("size"):
          fmt.size = std::atoi(attr.value.c_str());
          fmt.relative = (attr.value.back() == '%');
      }
    }

    if (! fmt.size)
      throw ConfigError(formatParser.text, "Missing column size");

    result.push_back(std::move(fmt));
  }

  return result;
}

//"<text fg='black'>&lt;&lt; </text><title bold='on' fg='yellow' /><text fg='black'> &gt;&gt;</text>";
static InfoLineFormat parse_infoline_format(ConstChars s) {
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
      using pack = StringPack::AlnumNoCase;
      switch (pack(attr.name)) {
        case pack("fg"): fmt.fg = parse_color(attr.value); break;
        case pack("bg"): fmt.bg = parse_color(attr.value); break;
        default:         fmt.attributes |= parse_attribute(attr.name);
      }
    }

    result.push_back(std::move(fmt));
  }

  return result;
}

/* ============================================================================
 * End of option parsing functions
 * ==========================================================================*/

static inline void check_arg_count(const Config::string_array& args, size_t min, size_t max) {
  if (args.size() < min) throw ConfigError("Missing arguments");
  if (args.size() > max) throw ConfigError("Too many arguments");
}

#include "config/options.define.cpp"

void Config :: init() {
#include "config/options.initialize.cpp"
}

void Config :: set(const string_array& args) {
  check_arg_count(args, 2, 2);
  const char* option = args[0];
  const char* value  = args[1];

  try {
    switch (Hash::djb2(option)) {
#include "config/options.set.cpp"
    default: throw ConfigError("unknown option");
    }
  } catch (const std::exception &e) {
    throw ConfigError(option, e.what());
  }
}

void Config :: color(Theme& theme, const string_array& args) {
  check_arg_count(args, 1, INT_MAX);

  Theme::Definition* def = theme.get(args[0]);
  if (! def)
    throw ConfigError(args[0], "Invalid theme element");

  def->fg = parse_color(args[1]);
  if (args.size() > 2) {
    def->bg = parse_color(args[2]);
    def->attributes = 0;
    for (auto i = args.size(); --i >= 3;)
      def->attributes |= parse_attribute(args[i]);
  }
}

void Config :: bind(const string_array& args) {
  check_arg_count(args, 3, 3);
}

void Config :: unbind(const string_array& args) {
  check_arg_count(args, 1, 1);
}

void Config :: unbind_all(const string_array& args) {
  check_arg_count(args, 1, 1);
  std::memset(Bindings::pad,      0, sizeof(Bindings::pad));
  std::memset(Bindings::global,   0, sizeof(Bindings::global));
  std::memset(Bindings::playlist, 0, sizeof(Bindings::playlist));
}

void Config :: read(const char* file) {
  auto fh = CFile::open(file, "r");
  char line[8192];
  std::vector<std::string> splitted;
  const char* c_args[64]; // XXX
  unsigned int no = 0;

  try {
    while (fh.gets(line, sizeof(line))) {
      line[std::max(size_t(1), std::strlen(line)) - 1] = '\0';

      ++no;
      ShellSplit::split(line, splitted);
      if (! splitted.size() || splitted[0][0] == '#')
        continue;

      for (size_t n = 0; n < splitted.size(); ++n)
        c_args[n] = splitted[n].c_str();

      ArrayView<const char*> args(c_args + 1, splitted.size() - 1);

      using pack = StringPack::AlnumNoCase;
      switch (pack(c_args[0])) {
        case pack("set"):          set(args);               break;
        case pack("color"):        color(THEME_8,    args); break;
        case pack("color_256"):    color(THEME_256,  args); break;
        case pack("color_mono"):   color(THEME_MONO, args); break;
        case pack("bind"):         bind(args);              break;
        case pack("unbind"):       unbind(args);            break;
        case pack("unbind_all"):   unbind_all(args);        break;
        default: throw ConfigError("unknown command");
      }
    }
  }
  catch (const std::exception &e) {
    char fname[PATH_MAX+16];
    std::snprintf(fname, sizeof(fname), "%s:%u", file, no);
    throw ConfigError(fname, c_args[0], e.what());
  }

  if (fh.error())
    throw std::system_error(EIO, std::generic_category());
}

#ifdef TEST_CONFIG
#include <lib/test.hpp>
#include "ui/colors.hpp"
int main() {
  TEST_BEGIN();

  Config::init();

  // === Primitives ===
  assert(parse_int("1") == 1);
  except(parse_int(""));
  except(parse_int("-"));
  except(parse_int("1a"));
  except(parse_int("a1"));

  assert(parse_bool("true") == true);
  assert(parse_bool("false") == false);
  except(parse_bool(""));
  except(parse_bool("-"));

  assert(parse_char("c") == 'c');
  except(parse_char(""));
  except(parse_char("12"));

  // === Objects ===
  { auto c = parse_tabs_widgets("  help , INFO ");
    assert(c[0] == TabWidgets::HELP);
    assert(c[1] == TabWidgets::INFO);
    assert(c[2] == TabWidgets::NONE);
  }

  { auto c = parse_main_widgets("  infoline , PROGRESSBAR ");
    assert(c[0] == MainWidgets::INFOLINE);
    assert(c[1] == MainWidgets::PROGRESSBAR);
    assert(c[2] == MainWidgets::NONE);
  }

  PlaylistColumns c;
  c = parse_playlist_columns("number{fg=blue bg=magenta size=3} artist{size=10%}");
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
  fmt = parse_infoline_format("'>>'{fg=blue bg=magenta bold}");
  assert(fmt.size()         == 1);
  assert(fmt[0].text        == ">>");
  assert(fmt[0].fg          == COLOR_BLUE);
  assert(fmt[0].bg          == COLOR_MAGENTA);
  assert(fmt[0].attributes  == A_BOLD);

  TEST_END();
}
#endif
