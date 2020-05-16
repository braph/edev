#ifndef UI_COLORS_HPP
#define UI_COLORS_HPP

#include CURSES_INC

#include <array>
#include <string>

namespace UI {

class Color {
public:
  struct ParseResult {
    short color;
    bool  ok;
    inline operator short() const noexcept { return color; }
  };

  static ParseResult parse(const std::string&)  noexcept;
  static std::string to_string(short)           noexcept;
private:
  struct mapping { const char* name; short value; };
  static mapping colors[];
};

class Attribute {
public:
  struct ParseResult {
    unsigned int attribute;
    bool         ok;
    inline operator unsigned int() const noexcept { return attribute; }
  };

  static ParseResult parse(const std::string&)  noexcept;
  static std::string to_string(unsigned int)    noexcept;
private:
  struct mapping { const char* name; unsigned int value; };
  static mapping attributes[];
};

class Colors {
public:
  static void reset()                         noexcept;
  static int create_color_pair(short, short)  noexcept;
  static unsigned int set(short fg, short bg = -1, unsigned int attributes = 0) noexcept;
private:
  struct color_pair { short fg; short bg; };
  static std::array<color_pair, 256> color_pairs;
  static int last_id;
};

} // namespace UI

#endif
