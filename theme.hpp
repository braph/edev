#ifndef THEME_HPP
#define THEME_HPP

#include <string>

class Theme {
public:
  struct Definition {
    short fg, bg;
    unsigned int attributes;

    Definition(short fg = -2, short bg = -2, unsigned int attributes = 0)
      : fg(fg), bg(bg), attributes(attributes)
    {
    }
  };

  enum ThemeID {
    THEME_MONO,
    THEME_8,
    THEME_256,
    THEMEID_ENUM_LAST
  };

  enum ElementID {
    DEFAULT,
    URL,

    INFO_HEAD,
    INFO_TAG,
    INFO_VALUE,
    INFO_DESCRIPTION,
    INFO_DOWNLOAD_FILE,
    INFO_DOWNLOAD_PERCENT,
    INFO_DOWNLOAD_ERROR,

    PROGRESSBAR_PROGRESS,
    PROGRESSBAR_REST,

    TABBAR_SELECTED,
    TABBAR_UNSELECTED,

    LIST_ITEM_EVEN,
    LIST_ITEM_ODD,
    LIST_ITEM_SELECTION,

    INFOLINE_POSITION,
    INFOLINE_STATE,

    HELP_WIDGET_NAME,
    HELP_KEY_NAME,
    HELP_COMMAND_NAME,
    HELP_COMMAND_DESC,

    ELEMENTID_ENUM_LAST
  };

  static ThemeID current;

  static bool set(ThemeID, const std::string&, short, short, unsigned int);
  static unsigned int get(ElementID);
  static void loadTheme(ThemeID);
  static void loadThemeByColors(int);

private:
  static Definition themes[THEMEID_ENUM_LAST][ELEMENTID_ENUM_LAST];
  static unsigned int loaded[ELEMENTID_ENUM_LAST];
};

#endif
