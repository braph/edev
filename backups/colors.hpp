#ifndef _COLORS_HPP
#define _COLORS_HPP

#include <map>
#include <string>
#include <curses.h>

namespace UI {
  class Colors {
    public:
      std::map<std::string, int> COLORS = {
        {"none",      -1},
        {"white",     COLOR_WHITE},
        {"black",     COLOR_BLACK},
        {"red",       COLOR_RED},
        {"blue",      COLOR_BLUE},
        {"cyan",      COLOR_CYAN},
        {"green",     COLOR_GREEN},
        {"yellow",    COLOR_YELLOW},
        {"magenta",   COLOR_MAGENTA}
      };

      std::map<std::string, int> ATTRIBUTES = {
        {"bold",      A_BOLD},
        {"blink",     A_BLINK},
        {"standout",  A_STANDOUT},
        {"underline", A_UNDERLINE}
      };

      std::map<int, std::map<int, int> > cached;
      std::map<std::string, int> aliases;

      int defaultFG = -1;
      int defaultBG = -1;
      int id;

      void start();
      void reset();
      int  getColorByName(const std::string&);
      int  getAttrByName(const std::string&);
      void setDefaultFG(const std::string&);
      void setDefaultBG(const std::string&);
      int  init_pair_cached(const std::string&, const std::string&);
      int  set(const std::string&, const std::string&, const std::string&);
      int  get(const std::string&);
  };
}

#endif
