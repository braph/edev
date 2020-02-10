#ifndef _COLORS_HPP
#define _COLORS_HPP

#include <map>
#include <string>
#include <cstdint>
#include <curses.h>

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

      static std::string to_string(int);
      static int parse(const std::string&);
  };

  class Colors {
    private:
      static std::map<int32_t, int> color_pairs; // = int16_t(fg) AND int16_t(bg)
      static int id;
    public:
      static int create_color_pair(short, short);
      static int set(short fg, short bg = -1, int attr = 0);
  };
}

#endif
