#ifndef _COLORS_HPP
#define _COLORS_HPP

#include <curses.h>

#include <vector>
#include <utility>
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
  static int set(short fg, short bg = -1, unsigned int attributes = 0);
private:
  static std::vector<std::pair<int32_t, int32_t>> color_pairs;
  static int id;
};

} // namespace UI

#endif
