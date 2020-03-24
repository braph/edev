#include "theme.hpp"
#include "ui/colors.hpp"

#include <stdexcept>

// Undef, just to be sure
#undef DEFAULT
#undef WHITE
#undef BLACK
#undef RED
#undef BLUE
#undef CYAN
#undef GREEN
#undef YELLOW
#undef MAGENTA

#define DEFAULT    -2
#define WHITE      COLOR_WHITE
#define BLACK      COLOR_BLACK
#define RED        COLOR_RED
#define BLUE       COLOR_BLUE
#define CYAN       COLOR_CYAN
#define GREEN      COLOR_GREEN
#define YELLOW     COLOR_YELLOW
#define MAGENTA    COLOR_MAGENTA

Theme::ThemeID Theme :: current;
unsigned int   Theme :: loaded[ELEMENTID_ENUM_LAST];

#define _ Theme :: Definition
Theme::Definition Theme :: themes[THEMEID_ENUM_LAST][ELEMENTID_ENUM_LAST] = {
  { // ========================= Mono (no colors) =============================
    /* DEFAULT                */ _(-1, -1                       ),
    /* URL                    */ _(DEFAULT, DEFAULT, A_UNDERLINE),

    /* INFO_HEAD              */ _(                             ),
    /* INFO_TAG               */ _(                             ),
    /* INFO_VALUE             */ _(                             ),
    /* INFO_DESCRIPTION       */ _(                             ),
    /* INFO_DOWNLOAD_FILE     */ _(                             ),
    /* INFO_DOWNLOAD_PERCENT  */ _(                             ),
    /* INFO_DOWNLOAD_ERROR    */ _(                             ),

    /* PROGRESSBAR_PROGRESS   */ _(                             ),
    /* PROGRESSBAR_REST       */ _(                             ),

    /* TABBAR_SELECTED        */ _(DEFAULT, DEFAULT, A_BOLD     ),
    /* TABBAR_UNSELECTED      */ _(                             ),

    /* LIST_ITEM_EVEN         */ _(                             ),
    /* LIST_ITEM_ODD          */ _(                             ),
    /* LIST_ITEM_SELECTION    */ _(                             ),

    /* INFOLINE_POSITION      */ _(                             ),
    /* INFOLINE_STATE         */ _(                             ),

    /* HELP_WIDGET_NAME       */ _(DEFAULT, DEFAULT, A_BOLD     ),
    /* HELP_KEY_NAME          */ _(                             ),
    /* HELP_COMMAND_NAME      */ _(                             ),
    /* HELP_COMMAND_DESC      */ _(                             )
  },
  { // ========================= 8 Colors (default) ===========================
    /* DEFAULT                */ _(-1, -1                       ),
    /* URL                    */ _(MAGENTA, DEFAULT, A_UNDERLINE),

    /* INFO_HEAD              */ _(BLUE, DEFAULT, A_BOLD        ),
    /* INFO_TAG               */ _(BLUE                         ),
    /* INFO_VALUE             */ _(MAGENTA                      ),
    /* INFO_DESCRIPTION       */ _(BLUE                         ),
    /* INFO_DOWNLOAD_FILE     */ _(BLUE                         ),
    /* INFO_DOWNLOAD_PERCENT  */ _(MAGENTA                      ),
    /* INFO_DOWNLOAD_ERROR    */ _(RED                          ),

    /* PROGRESSBAR_PROGRESS   */ _(BLUE                         ),
    /* PROGRESSBAR_REST       */ _(BLACK                        ),

    /* TABBAR_SELECTED        */ _(BLUE                         ),
    /* TABBAR_UNSELECTED      */ _(WHITE                        ),

    /* LIST_ITEM_EVEN         */ _(BLUE                         ),
    /* LIST_ITEM_ODD          */ _(BLUE                         ),
    /* LIST_ITEM_SELECTION    */ _(MAGENTA                      ),

    /* INFOLINE_POSITION      */ _(MAGENTA                      ),
    /* INFOLINE_STATE         */ _(CYAN                         ),

    /* HELP_WIDGET_NAME       */ _(BLUE, DEFAULT, A_BOLD        ),
    /* HELP_KEY_NAME          */ _(BLUE                         ),
    /* HELP_COMMAND_NAME      */ _(MAGENTA                      ),
    /* HELP_COMMAND_DESC      */ _(YELLOW                       )
  },
  { // ========================= 256 Colors ===================================
    /* DEFAULT                */ _(WHITE, -1                    ), // 233
    /* URL                    */ _(97, DEFAULT, A_UNDERLINE     ),

    /* INFO_HEAD              */ _(32, DEFAULT, A_BOLD          ),
    /* INFO_TAG               */ _(74                           ),
    /* INFO_VALUE             */ _(67                           ),
    /* INFO_DESCRIPTION       */ _(67                           ),
    /* INFO_DOWNLOAD_FILE     */ _(75                           ),
    /* INFO_DOWNLOAD_PERCENT  */ _(68                           ),
    /* INFO_DOWNLOAD_ERROR    */ _(RED                          ),

    /* PROGRESSBAR_PROGRESS   */ _(23                           ),
    /* PROGRESSBAR_REST       */ _(BLACK                        ),

    /* TABBAR_SELECTED        */ _(75                           ),
    /* TABBAR_UNSELECTED      */ _(250                          ),

    /* LIST_ITEM_EVEN         */ _(26                           ),
    /* LIST_ITEM_ODD          */ _(25                           ),
    /* LIST_ITEM_SELECTION    */ _(97                           ),

    /* INFOLINE_POSITION      */ _(97                           ),
    /* INFOLINE_STATE         */ _(37                           ),

    /* HELP_WIDGET_NAME       */ _(33, DEFAULT, A_BOLD          ),
    /* HELP_KEY_NAME          */ _(75                           ),
    /* HELP_COMMAND_NAME      */ _(68                           ),
    /* HELP_COMMAND_DESC      */ _(29                           )
  }
};
#undef _
#undef DEFAULT
#undef WHITE
#undef BLACK
#undef RED
#undef BLUE
#undef CYAN
#undef GREEN
#undef YELLOW
#undef MAGENTA

bool Theme :: set(ThemeID theme, const std::string &name, short fg, short bg, unsigned int attributes) {
  const char* names[ELEMENTID_ENUM_LAST] = {
    "default",
    "url",

    "info.head",
    "info.tag",
    "info.value",
    "info.description",
    "info.file",
    "info.download_percent",
    "info.download_error",

    "progressbar.progress",
    "progressbar.rest",

    "tabbar.selected",
    "tabbar.unselected",

    "list.item_even",
    "list.item_odd",
    "list.item_selection",

    "infoline.position",
    "infoline.state",

    "help.widget_name",
    "help.key_name",
    "help.command_name",
    "help.command_desc",
  };

  for (size_t i = 0; i < THEMEID_ENUM_LAST; ++i)
    if (name == names[i]) {
      themes[theme][i] = Theme::Definition(fg, bg, attributes);
      return true;
    }

  return false;
}

unsigned int Theme :: get(ElementID id) {
  return loaded[id];
}

void Theme :: loadTheme(ThemeID theme) {
  current = theme;
  Theme::Definition fallback = themes[theme][DEFAULT];

  for (size_t i = 0; i < ELEMENTID_ENUM_LAST; ++i) {
    Theme::Definition td = themes[theme][i];
    loaded[i] = UI::Colors::set(
      (td.fg == -2 ? fallback.fg : td.fg),
      (td.bg == -2 ? fallback.bg : td.bg),
      td.attributes
    );
  }
}

void Theme :: loadThemeByColors(int colors) {
  loadTheme((colors >= 256 ? THEME_256 : (colors >= 8 ? THEME_8 : THEME_MONO)));
}

#ifdef TEST_THEME
int main() { }
#endif

