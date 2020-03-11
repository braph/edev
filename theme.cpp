#include "theme.hpp"
#include "colors.hpp"

#include <stdexcept>

#define defualt    -2
#define white      COLOR_WHITE
#define black      COLOR_BLACK
#define red        COLOR_RED
#define blue       COLOR_BLUE
#define cyan       COLOR_CYAN
#define green      COLOR_GREEN
#define yellow     COLOR_YELLOW
#define magenta    COLOR_MAGENTA

int          Theme :: current;
unsigned int Theme :: loaded[THEME_ID_COUNT];

#define _ Theme :: Definition
Theme::Definition Theme :: themes[3][THEME_ID_COUNT] = {
  { // ========================= Mono (no colors) ===============
    /* DEFAULT                */ _(-1, -1                       ),
    /* URL                    */ _(defualt, defualt, A_UNDERLINE),

    /* INFO_HEAD              */ _(                             ),
    /* INFO_TAG               */ _(                             ),
    /* INFO_VALUE             */ _(                             ),
    /* INFO_DESCRIPTION       */ _(                             ),
    /* INFO_DOWNLOAD_FILE     */ _(                             ),
    /* INFO_DOWNLOAD_PERCENT  */ _(                             ),
    /* INFO_DOWNLOAD_ERROR    */ _(                             ),

    /* PROGRESSBAR_PROGRESS   */ _(                             ),
    /* PROGRESSBAR_REST       */ _(                             ),

    /* TABBAR_SELECTED        */ _(defualt, defualt, A_BOLD     ),
    /* TABBAR_UNSELECTED      */ _(                             ),

    /* LIST_ITEM_EVEN         */ _(                             ),
    /* LIST_ITEM_ODD          */ _(                             ),
    /* LIST_ITEM_SELECTION    */ _(                             ),

    /* PLAYINGINFO_POSITION   */ _(                             ),
    /* PLAYINGINFO_STATE      */ _(                             ),

    /* HELP_WIDGET_NAME       */ _(defualt, defualt, A_BOLD     ),
    /* HELP_KEY_NAME          */ _(                             ),
    /* HELP_COMMAND_NAME      */ _(                             ),
    /* HELP_COMMAND_DESC      */ _(                             )
  },
  { // ========================= 8 Colors (default) =============
    /* DEFAULT                */ _(-1, -1                       ),
    /* URL                    */ _(magenta, defualt, A_UNDERLINE),

    /* INFO_HEAD              */ _(blue, defualt, A_BOLD        ),
    /* INFO_TAG               */ _(blue                         ),
    /* INFO_VALUE             */ _(magenta                      ),
    /* INFO_DESCRIPTION       */ _(blue                         ),
    /* INFO_DOWNLOAD_FILE     */ _(blue                         ),
    /* INFO_DOWNLOAD_PERCENT  */ _(magenta                      ),
    /* INFO_DOWNLOAD_ERROR    */ _(red                          ),

    /* PROGRESSBAR_PROGRESS   */ _(blue                         ),
    /* PROGRESSBAR_REST       */ _(black                        ),

    /* TABBAR_SELECTED        */ _(blue                         ),
    /* TABBAR_UNSELECTED      */ _(white                        ),

    /* LIST_ITEM_EVEN         */ _(blue                         ),
    /* LIST_ITEM_ODD          */ _(blue                         ),
    /* LIST_ITEM_SELECTION    */ _(magenta                      ),

    /* PLAYINGINFO_POSITION   */ _(magenta                      ),
    /* PLAYINGINFO_STATE      */ _(cyan                         ),

    /* HELP_WIDGET_NAME       */ _(blue, defualt, A_BOLD        ),
    /* HELP_KEY_NAME          */ _(blue                         ),
    /* HELP_COMMAND_NAME      */ _(magenta                      ),
    /* HELP_COMMAND_DESC      */ _(yellow                       )
  },
  { // ========================= 256 Colors =====================
    /* DEFAULT                */ _(white, -1                    ), // 233
    /* URL                    */ _(97, defualt, A_UNDERLINE     ),

    /* INFO_HEAD              */ _(32, defualt, A_BOLD          ),
    /* INFO_TAG               */ _(74                           ),
    /* INFO_VALUE             */ _(67                           ),
    /* INFO_DESCRIPTION       */ _(67                           ),
    /* INFO_DOWNLOAD_FILE     */ _(75                           ),
    /* INFO_DOWNLOAD_PERCENT  */ _(68                           ),
    /* INFO_DOWNLOAD_ERROR    */ _(red                          ),

    /* PROGRESSBAR_PROGRESS   */ _(23                           ),
    /* PROGRESSBAR_REST       */ _(black                        ),

    /* TABBAR_SELECTED        */ _(75                           ),
    /* TABBAR_UNSELECTED      */ _(250                          ),

    /* LIST_ITEM_EVEN         */ _(26                           ),
    /* LIST_ITEM_ODD          */ _(25                           ),
    /* LIST_ITEM_SELECTION    */ _(97                           ),

    /* PLAYINGINFO_POSITION   */ _(97                           ),
    /* PLAYINGINFO_STATE      */ _(37                           ),

    /* HELP_WIDGET_NAME       */ _(33, defualt, A_BOLD          ),
    /* HELP_KEY_NAME          */ _(75                           ),
    /* HELP_COMMAND_NAME      */ _(68                           ),
    /* HELP_COMMAND_DESC      */ _(29                           )
  }
};
#undef _

void Theme :: set(int theme, const std::string &name, short fg, short bg, unsigned int attributes) {
  const char* names[THEME_ID_COUNT] = {
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

    "playinginfo.position",
    "playinginfo.state",

    "help.widget_name",
    "help.key_name",
    "help.command_name",
    "help.command_desc",
  };

  for (size_t i = 0; i < THEME_ID_COUNT; ++i)
    if (name == names[i]) {
      themes[theme][i] = Theme::Definition(fg, bg, attributes);
      return;
    }

  throw std::invalid_argument(name + ": invalid theme element");
}

unsigned int Theme :: get(ThemeID id) {
  return loaded[id];
}

void Theme :: loadTheme(int theme) {
  current = theme;
  int theme_idx = (theme >= 256 ? 2 : (theme >= 8 ? 1 : 0));

  Theme::Definition fallback = themes[theme_idx][DEFAULT];

  for (size_t i = 0; i < THEME_ID_COUNT; ++i) {
    Theme::Definition td = themes[theme_idx][i];
    loaded[i] = UI::Colors::set(
      (td.fg == -2 ? fallback.fg : td.fg),
      (td.bg == -2 ? fallback.bg : td.bg),
      td.attributes
    );
  }
}

#ifdef TEST_THEME
#include "test.hpp"
int main() {
}
#endif

