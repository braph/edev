#ifndef _COLORS_HPP
#define _COLORS_HPP

#include <map>
#include <string>
#include <cstdint>
#include <curses.h>

namespace UI {
  class Color {
    private:
      static std::map<std::string, short> colors;
    public:
      static void init();
      static std::string to_string(short);
      static short parse(const std::string&);
  };

  class Attribute {
    private:
      static std::map<std::string, int> attributes;
    public:
      static void init();
      static const std::string& to_string(int);
      static int parse(const std::string&);
  };

  class Colors {
    private:
      static std::map<int32_t, int> color_pairs; // = int16_t(fg) AND int16_t(bg)
      static std::map<std::string, int> aliases;
      static int id;
    public:
      static void init();
      static int  create_color_pair(short, short);
      static int  set(const std::string&, short, short, int);
      static int  get(const std::string&);
  };
}

#endif
