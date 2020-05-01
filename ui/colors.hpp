#ifndef UI_COLORS_HPP
#define UI_COLORS_HPP

#include CURSES_INC

#include <vector>
#include <string>

namespace UI {

class Color {
public:
  static std::string to_string(short);
  static short parse(const std::string&, short on_error_return) noexcept;
private:
  struct mapping { const char* name; short value; };
  static mapping colors[];
};

class Attribute {
public:
  static std::string to_string(unsigned int);
  static unsigned int parse(const std::string&) noexcept;
private:
  struct mapping { const char* name; unsigned int value; };
  static mapping attributes[];
};

class Colors {
public:
  static int create_color_pair(short, short);
  static unsigned int set(short fg, short bg = -1, unsigned int attributes = 0);
  static void reset() noexcept;
private:
  static std::vector<unsigned int> color_pairs;
  static int last_id;
};

} // namespace UI

#endif
