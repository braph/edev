#ifndef _THEME_CPP
#define _THEME_CPP

#include <map>

// TODO: current as getter?

namespace Ektoplayer {
  class ThemeDefinition {
    public:
      short fg, bg;
      int   attributes;

      ThemeDefinition(short _fg = -2, short _bg = -2, int _attributes = 0)
        : fg(_fg), bg(_bg), attributes(_attributes)
      {
      }
  };

  class Theme {
    private:
      static std::map<std::string, ThemeDefinition> themes[3];
    public:
      static unsigned int current; // 0 | 8 | 256

      static void init();
      static void set(unsigned int, const std::string&, short, short, int);
      static void loadTheme(unsigned int);
  };
}

#endif
