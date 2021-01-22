#include "theme.hpp"
#include "ui/colors.hpp"

#include <lib/hash.hpp>

#define COLOR_NONE    -1
#define COLOR_DEFAULT -2

LoadedColors colors;

Theme THEME_MONO = {
  /* default               */ {COLOR_NONE, COLOR_NONE},
  /* url                   */ {COLOR_DEFAULT, COLOR_DEFAULT, A_UNDERLINE},

  /* info_head             */ {},
  /* info_tag              */ {},
  /* info_value            */ {},
  /* info_description      */ {},
  /* info_download_file    */ {},
  /* info_download_percent */ {},
  /* info_download_error   */ {},

  /* progressbar_progress  */ {},
  /* progressbar_rest      */ {},

  /* tabbar_selected       */ {COLOR_DEFAULT, COLOR_DEFAULT, A_BOLD},
  /* tabbar_unselected     */ {},

  /* list_item_even        */ {},
  /* list_item_odd         */ {},
  /* list_item_selection   */ {},

  /* infoline_position     */ {},
  /* infoline_state        */ {},

  /* help_widget_name      */ {COLOR_DEFAULT, COLOR_DEFAULT, A_BOLD},
  /* help_key_name         */ {},
  /* help_command_name     */ {},
  /* help_command_desc     */ {}
};

Theme THEME_8 = {
  /* default               */ {COLOR_NONE, COLOR_NONE},
  /* url                   */ {COLOR_MAGENTA, COLOR_DEFAULT, A_UNDERLINE},

  /* info_head             */ {COLOR_BLUE, COLOR_DEFAULT, A_BOLD},
  /* info_tag              */ {COLOR_BLUE},
  /* info_value            */ {COLOR_MAGENTA},
  /* info_description      */ {COLOR_BLUE},
  /* info_download_file    */ {COLOR_BLUE},
  /* info_download_percent */ {COLOR_MAGENTA},
  /* info_download_error   */ {COLOR_RED},

  /* progressbar_progress  */ {COLOR_BLUE},
  /* progressbar_rest      */ {COLOR_BLACK},

  /* tabbar_selected       */ {COLOR_BLUE},
  /* tabbar_unselected     */ {COLOR_WHITE},

  /* list_item_even        */ {COLOR_BLUE},
  /* list_item_odd         */ {COLOR_BLUE},
  /* list_item_selection   */ {COLOR_MAGENTA},

  /* infoline_position     */ {COLOR_MAGENTA},
  /* infoline_state        */ {COLOR_CYAN},

  /* help_widget_name      */ {COLOR_BLUE, COLOR_DEFAULT, A_BOLD},
  /* help_key_name         */ {COLOR_BLUE},
  /* help_command_name     */ {COLOR_MAGENTA},
  /* help_command_desc     */ {COLOR_YELLOW}
};

Theme THEME_256 = {
  /* default               */ {COLOR_WHITE, COLOR_NONE}, // 233
  /* url                   */ {97, COLOR_DEFAULT, A_UNDERLINE},

  /* info_head             */ {32, COLOR_DEFAULT, A_BOLD},
  /* info_tag              */ {74},
  /* info_value            */ {67},
  /* info_description      */ {67},
  /* info_download_file    */ {75},
  /* info_download_percent */ {68},
  /* info_download_error   */ {COLOR_RED},

  /* progressbar_progress  */ {23},
  /* progressbar_rest      */ {COLOR_BLACK},

  /* tabbar_selected       */ {75},
  /* tabbar_unselected     */ {250},

  /* list_item_even        */ {26},
  /* list_item_odd         */ {25},
  /* list_item_selection   */ {97},

  /* infoline_position     */ {97},
  /* infoline_state        */ {37},

  /* help_widget_name      */ {33, COLOR_DEFAULT, A_BOLD},
  /* help_key_name         */ {75},
  /* help_command_name     */ {68},
  /* help_command_desc     */ {29}
};

Theme::Definition* Theme :: get(const std::string& name) noexcept {
#define THEME_HPP__CASE_AND_RETURN(NAME, STRING) case Hash::djb2(STRING): return &NAME;
  switch (Hash::djb2(name)) { THEME_HPP__FOREACH_ELEMENT(THEME_HPP__CASE_AND_RETURN) }
  return NULL;
}

static attr_t theme_set_color(Theme::Definition& def, Theme::Definition& default_) {
  return UI::Colors::set(
      (def.fg == -2 ? default_.fg : def.fg),
      (def.bg == -2 ? default_.bg : def.bg))
      | def.attributes;
}

void Theme :: load_theme(LoadedColors& colors) noexcept {
  UI::Colors::reset();

#define THEME_HPP__SET_COLOR(NAME, _) colors.NAME = theme_set_color(NAME, default_);
  THEME_HPP__FOREACH_ELEMENT(THEME_HPP__SET_COLOR)
}

void load_theme_by_colors(int num_colors, LoadedColors& colors) noexcept {
  if (num_colors >= 256)    THEME_256.load_theme(colors);
  else if (num_colors >= 8) THEME_8.load_theme(colors);
  else                      THEME_MONO.load_theme(colors);
}
