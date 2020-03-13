#ifndef UI_COLORS_HPP
#define UI_COLORS_HPP

#include CURSES_INC

#include <vector>
#include <string>
#include <cstdint>

namespace UI {

class Color {
public:
  struct mapping { const char* name; short value; };
  static mapping colors[];

  static std::string to_string(short);
  static short parse(const std::string&);
};

class Attribute {
public:
  struct mapping { const char* name; unsigned int value; };
  static mapping attributes[];

  static std::string to_string(unsigned int);
  static unsigned int parse(const std::string&);
};

class Colors {
public:
  static int create_color_pair(short, short);
  static unsigned int set(short fg, short bg = -1, unsigned int attributes = 0);
private:
  struct pair_id { short fg; short bg; int id; };
  static std::vector<pair_id> color_pairs;
  static int id;
};

} // namespace UI

#endif
