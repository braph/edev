#ifndef UI_COLORS_HPP
#define UI_COLORS_HPP

#include CURSES_INC

#include <array>
#include <string>
#include <climits>

namespace UI {

class Color {
public:
  enum : short { Invalid = SHRT_MIN };

  static short       parse(const std::string&)  noexcept;
  static std::string to_string(short)           noexcept;
private:
  struct mapping { const char* name; short value; };
  static mapping colors[];
};

class Attribute {
public:
  enum : unsigned { Invalid = UINT_MAX };

  static unsigned     parse(const std::string&) noexcept;
  static std::string  to_string(unsigned)       noexcept;
private:
  struct mapping { const char* name; unsigned value; };
  static mapping attributes[];
};

class Colors {
public:
  static void reset()                          noexcept;
  static int  create_color_pair(short, short)  noexcept;
  static unsigned set(short fg, short bg = -1, unsigned attributes = 0) noexcept;
private:
  struct color_pair { short fg; short bg; };
  static std::array<color_pair, 256> color_pairs;
  static int last_id;
};

} // namespace UI

#endif
