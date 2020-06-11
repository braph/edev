#ifndef THEME_HPP
#define THEME_HPP

#include <string>

#define THEME_ELEMENT_IDS                            \
  X(DEFAULT,                "default")               \
  X(URL,                    "url")                   \
                                                     \
  X(INFO_HEAD,              "info.head")             \
  X(INFO_TAG,               "info.tag")              \
  X(INFO_VALUE,             "info.value")            \
  X(INFO_DESCRIPTION,       "info.description")      \
  X(INFO_DOWNLOAD_FILE,     "info.file")             \
  X(INFO_DOWNLOAD_PERCENT,  "info.download_percent") \
  X(INFO_DOWNLOAD_ERROR,    "info.download_error")   \
                                                     \
  X(PROGRESSBAR_PROGRESS,   "progressbar.progress")  \
  X(PROGRESSBAR_REST,       "progressbar.rest")      \
                                                     \
  X(TABBAR_SELECTED,        "tabbar.selected")       \
  X(TABBAR_UNSELECTED,      "tabbar.unselected")     \
                                                     \
  X(LIST_ITEM_EVEN,         "list.item_even")        \
  X(LIST_ITEM_ODD,          "list.item_odd")         \
  X(LIST_ITEM_SELECTION,    "list.item_selection")   \
                                                     \
  X(INFOLINE_POSITION,      "infoline.position")     \
  X(INFOLINE_STATE,         "infoline.state")        \
                                                     \
  X(HELP_WIDGET_NAME,       "help.widget_name")      \
  X(HELP_KEY_NAME,          "help.key_name")         \
  X(HELP_COMMAND_NAME,      "help.command_name")     \
  X(HELP_COMMAND_DESC,      "help.command_desc")     \

class Theme {
public:
  struct Definition {
    short fg;
    short bg;
    unsigned int attributes;

    Definition(short fg_ = -2, short bg_ = -2, unsigned int attributes_ = 0) noexcept
      : fg(fg_), bg(bg_), attributes(attributes_)
    {
    }
  };

  enum class ThemeID {
    THEME_MONO,
    THEME_8,
    THEME_256,
    COUNT
  };

#define X(ENUM, STRING) ENUM,
  enum class ElementID {
    THEME_ELEMENT_IDS
    COUNT
  };
#undef X

  static ThemeID current;

  static void set(ThemeID, ElementID, short, short, unsigned int) noexcept;
  static unsigned int get(ElementID)                              noexcept;
  static void loadTheme(ThemeID)                                  noexcept;
  static void loadThemeByColors(int)                              noexcept;
  static ElementID elementByString(const std::string&)            noexcept;

private:
  static Definition   themes[int(ThemeID::COUNT)][int(ElementID::COUNT)];
  static unsigned int loaded[int(ElementID::COUNT)];
};

#endif
