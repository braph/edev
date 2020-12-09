#include "theme.hpp"
#include "ui/colors.hpp"

#include <lib/hash.hpp>

#define COLOR_NONE    -1
#define COLOR_DEFAULT -2

ThemeID current_theme;
LoadedColors colors;
Theme themes[THEME_COUNT] = {
  { // ------------------------ Mono (no colors) ------------
    /* DEFAULT               */ {COLOR_NONE, COLOR_NONE},
    /* URL                   */ {COLOR_DEFAULT, COLOR_DEFAULT, A_UNDERLINE},
    /* INFO_HEAD             */ {},
    /* INFO_TAG              */ {},
    /* INFO_VALUE            */ {},
    /* INFO_DESCRIPTION      */ {},
    /* INFO_DOWNLOAD_FILE    */ {},
    /* INFO_DOWNLOAD_PERCENT */ {},
    /* INFO_DOWNLOAD_ERROR   */ {},
    /* PROGRESSBAR_PROGRESS  */ {},
    /* PROGRESSBAR_REST      */ {},
    /* TABBAR_SELECTED       */ {COLOR_DEFAULT, COLOR_DEFAULT, A_BOLD},
    /* TABBAR_UNSELECTED     */ {},
    /* LIST_ITEM_EVEN        */ {},
    /* LIST_ITEM_ODD         */ {},
    /* LIST_ITEM_SELECTION   */ {},
    /* INFOLINE_POSITION     */ {},
    /* INFOLINE_STATE        */ {},
    /* HELP_WIDGET_NAME      */ {COLOR_DEFAULT, COLOR_DEFAULT, A_BOLD},
    /* HELP_KEY_NAME         */ {},
    /* HELP_COMMAND_NAME     */ {},
    /* HELP_COMMAND_DESC     */ {}
  },
  { // ------------------------ 8 Colors (default) ----------
    /* DEFAULT               */ {COLOR_NONE, COLOR_NONE},
    /* URL                   */ {COLOR_MAGENTA, COLOR_DEFAULT, A_UNDERLINE},

    /* INFO_HEAD             */ {COLOR_BLUE, COLOR_DEFAULT, A_BOLD},
    /* INFO_TAG              */ {COLOR_BLUE},
    /* INFO_VALUE            */ {COLOR_MAGENTA},
    /* INFO_DESCRIPTION      */ {COLOR_BLUE},
    /* INFO_DOWNLOAD_FILE    */ {COLOR_BLUE},
    /* INFO_DOWNLOAD_PERCENT */ {COLOR_MAGENTA},
    /* INFO_DOWNLOAD_ERROR   */ {COLOR_RED},

    /* PROGRESSBAR_PROGRESS  */ {COLOR_BLUE},
    /* PROGRESSBAR_REST      */ {COLOR_BLACK},

    /* TABBAR_SELECTED       */ {COLOR_BLUE},
    /* TABBAR_UNSELECTED     */ {COLOR_WHITE},

    /* LIST_ITEM_EVEN        */ {COLOR_BLUE},
    /* LIST_ITEM_ODD         */ {COLOR_BLUE},
    /* LIST_ITEM_SELECTION   */ {COLOR_MAGENTA},

    /* INFOLINE_POSITION     */ {COLOR_MAGENTA},
    /* INFOLINE_STATE        */ {COLOR_CYAN},

    /* HELP_WIDGET_NAME      */ {COLOR_BLUE, COLOR_DEFAULT, A_BOLD},
    /* HELP_KEY_NAME         */ {COLOR_BLUE},
    /* HELP_COMMAND_NAME     */ {COLOR_MAGENTA},
    /* HELP_COMMAND_DESC     */ {COLOR_YELLOW}
  },
  { // ------------------------ 256 Colors ------------------
    /* DEFAULT               */ {COLOR_WHITE, COLOR_NONE}, // 233
    /* URL                   */ {97, COLOR_DEFAULT, A_UNDERLINE},

    /* INFO_HEAD             */ {32, COLOR_DEFAULT, A_BOLD},
    /* INFO_TAG              */ {74},
    /* INFO_VALUE            */ {67},
    /* INFO_DESCRIPTION      */ {67},
    /* INFO_DOWNLOAD_FILE    */ {75},
    /* INFO_DOWNLOAD_PERCENT */ {68},
    /* INFO_DOWNLOAD_ERROR   */ {COLOR_RED},

    /* PROGRESSBAR_PROGRESS  */ {23},
    /* PROGRESSBAR_REST      */ {COLOR_BLACK},

    /* TABBAR_SELECTED       */ {75},
    /* TABBAR_UNSELECTED     */ {250},

    /* LIST_ITEM_EVEN        */ {26},
    /* LIST_ITEM_ODD         */ {25},
    /* LIST_ITEM_SELECTION   */ {97},

    /* INFOLINE_POSITION     */ {97},
    /* INFOLINE_STATE        */ {37},

    /* HELP_WIDGET_NAME      */ {33, COLOR_DEFAULT, A_BOLD},
    /* HELP_KEY_NAME         */ {75},
    /* HELP_COMMAND_NAME     */ {68},
    /* HELP_COMMAND_DESC     */ {29}
  }
};

Theme::Definition* Theme :: get(const std::string& name) noexcept {
#define X(IDENTIFIER, STRING) case Hash::lose_lose(STRING): return &IDENTIFIER;
  switch (Hash::lose_lose(name)) { THEME_ELEMENT_IDS }
#undef X
  return NULL;
}

static unsigned int load_(Theme::Definition& def, Theme::Definition& default_) {
  return UI::Colors::set(
      (def.fg == -2 ? default_.fg : def.fg),
      (def.bg == -2 ? default_.bg : def.bg))
      | def.attributes;
}

void Theme :: load_theme(LoadedColors& colors) noexcept {
  UI::Colors::reset();

#define X(IDENTIFIER, _) colors.IDENTIFIER = load_(IDENTIFIER, default_);
  THEME_ELEMENT_IDS
#undef X
}

void load_theme_by_colors(int num_colors, LoadedColors& colors) noexcept {
  current_theme = (
    num_colors >= 256 ? THEME_256 :
    num_colors >= 8   ? THEME_8 :
    THEME_MONO
  );

  themes[current_theme].load_theme(colors);
}
