#ifndef _THEME_CPP
#define _THEME_CPP

#include <string>

struct ThemeDefinition {
  short fg, bg;
  unsigned int attributes;

  ThemeDefinition(short fg = -2, short bg = -2, unsigned int attributes = 0)
    : fg(fg), bg(bg), attributes(attributes)
  {
  }
};

class Theme {
public:
  enum ThemeID {
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

    PLAYINGINFO_POSITION,
    PLAYINGINFO_STATE,

    HELP_WIDGET_NAME,
    HELP_KEY_NAME,
    HELP_COMMAND_NAME,
    HELP_COMMAND_DESC,

    THEME_ID_COUNT
  };

  static int  current; // 0 | 8 | 256
  static void set(unsigned int, const std::string&, short, short, unsigned int);
  static int  get(ThemeID);
  static void loadTheme(int);

private:
  static ThemeDefinition themes[3][THEME_ID_COUNT];
  static int loaded[THEME_ID_COUNT];
};

#endif
