#include "theme.hpp"
#include "ui/colors.hpp"

#include <lib/hash.hpp>

enum {
  deflt   = -2,
  none    = -1,
  white   = COLOR_WHITE,
  black   = COLOR_BLACK,
  red     = COLOR_RED,
  blue    = COLOR_BLUE,
  cyan    = COLOR_CYAN,
  green   = COLOR_GREEN,
  yellow  = COLOR_YELLOW,
  magenta = COLOR_MAGENTA,
};

Theme::ThemeID    Theme :: current;
unsigned int      Theme :: loaded[int(ElementID::COUNT)];
Theme::Definition Theme :: themes[int(ThemeID::COUNT)][int(ElementID::COUNT)] = {
  { // ------------------------ Mono (no colors) ------------
    /* DEFAULT               */ {-1, -1                     },
    /* URL                   */ {deflt, deflt, A_UNDERLINE, },

    /* INFO_HEAD             */ {                           },
    /* INFO_TAG              */ {                           },
    /* INFO_VALUE            */ {                           },
    /* INFO_DESCRIPTION      */ {                           },
    /* INFO_DOWNLOAD_FILE    */ {                           },
    /* INFO_DOWNLOAD_PERCENT */ {                           },
    /* INFO_DOWNLOAD_ERROR   */ {                           },

    /* PROGRESSBAR_PROGRESS  */ {                           },
    /* PROGRESSBAR_REST      */ {                           },

    /* TABBAR_SELECTED       */ {deflt, deflt, A_BOLD       },
    /* TABBAR_UNSELECTED     */ {                           },

    /* LIST_ITEM_EVEN        */ {                           },
    /* LIST_ITEM_ODD         */ {                           },
    /* LIST_ITEM_SELECTION   */ {                           },

    /* INFOLINE_POSITION     */ {                           },
    /* INFOLINE_STATE        */ {                           },

    /* HELP_WIDGET_NAME      */ {deflt, deflt, A_BOLD       },
    /* HELP_KEY_NAME         */ {                           },
    /* HELP_COMMAND_NAME     */ {                           },
    /* HELP_COMMAND_DESC     */ {                           }
  },
  { // ------------------------ 8 Colors (default) ----------
    /* DEFAULT               */ {-1, -1                     },
    /* URL                   */ {magenta, deflt, A_UNDERLINE},

    /* INFO_HEAD             */ {blue, deflt, A_BOLD        },
    /* INFO_TAG              */ {blue                       },
    /* INFO_VALUE            */ {magenta                    },
    /* INFO_DESCRIPTION      */ {blue                       },
    /* INFO_DOWNLOAD_FILE    */ {blue                       },
    /* INFO_DOWNLOAD_PERCENT */ {magenta                    },
    /* INFO_DOWNLOAD_ERROR   */ {red                        },

    /* PROGRESSBAR_PROGRESS  */ {blue                       },
    /* PROGRESSBAR_REST      */ {black                      },

    /* TABBAR_SELECTED       */ {blue                       },
    /* TABBAR_UNSELECTED     */ {white                      },

    /* LIST_ITEM_EVEN        */ {blue                       },
    /* LIST_ITEM_ODD         */ {blue                       },
    /* LIST_ITEM_SELECTION   */ {magenta                    },

    /* INFOLINE_POSITION     */ {magenta                    },
    /* INFOLINE_STATE        */ {cyan                       },

    /* HELP_WIDGET_NAME      */ {blue, deflt, A_BOLD        },
    /* HELP_KEY_NAME         */ {blue                       },
    /* HELP_COMMAND_NAME     */ {magenta                    },
    /* HELP_COMMAND_DESC     */ {yellow                     }
  },
  { // ------------------------ 256 Colors ------------------
    /* DEFAULT               */ {white, -1                  }, // 233
    /* URL                   */ {97, deflt, A_UNDERLINE     },

    /* INFO_HEAD             */ {32, deflt, A_BOLD          },
    /* INFO_TAG              */ {74                         },
    /* INFO_VALUE            */ {67                         },
    /* INFO_DESCRIPTION      */ {67                         },
    /* INFO_DOWNLOAD_FILE    */ {75                         },
    /* INFO_DOWNLOAD_PERCENT */ {68                         },
    /* INFO_DOWNLOAD_ERROR   */ {red                        },

    /* PROGRESSBAR_PROGRESS  */ {23                         },
    /* PROGRESSBAR_REST      */ {black                      },

    /* TABBAR_SELECTED       */ {75                         },
    /* TABBAR_UNSELECTED     */ {250                        },

    /* LIST_ITEM_EVEN        */ {26                         },
    /* LIST_ITEM_ODD         */ {25                         },
    /* LIST_ITEM_SELECTION   */ {97                         },

    /* INFOLINE_POSITION     */ {97                         },
    /* INFOLINE_STATE        */ {37                         },

    /* HELP_WIDGET_NAME      */ {33, deflt, A_BOLD          },
    /* HELP_KEY_NAME         */ {75                         },
    /* HELP_COMMAND_NAME     */ {68                         },
    /* HELP_COMMAND_DESC     */ {29                         }
  }
};

Theme::ElementID Theme :: element_by_string(const std::string& name) noexcept {
#define X(ENUM, STRING) case Hash::lose_lose(STRING): return ElementID::ENUM;
  switch (Hash::lose_lose(name)) { THEME_ELEMENT_IDS }
  return ElementID::COUNT;
#undef X
}

void Theme :: set(ThemeID theme, ElementID element, short fg, short bg, unsigned int attributes) noexcept {
  themes[int(theme)][int(element)] = Theme::Definition(fg, bg, attributes);
}

unsigned int Theme :: get(ElementID id) noexcept {
  return loaded[int(id)];
}

void Theme :: load_theme(ThemeID theme) noexcept {
  UI::Colors::reset();

  current = theme;
  Theme::Definition fallback = themes[int(theme)][int(ElementID::DEFAULT)];

  for (int i = 0; i < int(ElementID::COUNT); ++i) {
    Theme::Definition td = themes[int(theme)][i];
    loaded[i] = UI::Colors::set(
      (td.fg == -2 ? fallback.fg : td.fg),
      (td.bg == -2 ? fallback.bg : td.bg),
      td.attributes
    );
  }
}

void Theme :: load_theme_by_colors(int colors) noexcept {
  load_theme(
    colors >= 256 ? ThemeID::THEME_256 :
    colors >= 8   ? ThemeID::THEME_8 :
    ThemeID::THEME_MONO);
}

#ifdef TEST_THEME
int main() { }
#endif

