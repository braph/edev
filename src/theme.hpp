#ifndef THEME_HPP
#define THEME_HPP

#include CURSES_INC

#include <string>

#define THEME_ELEMENT_IDS                            \
  X(default_,               "default")               \
  X(url,                    "url")                   \
                                                     \
  X(info_head,              "info.head")             \
  X(info_tag,               "info.tag")              \
  X(info_value,             "info.value")            \
  X(info_description,       "info.description")      \
  X(info_download_file,     "info.file")             \
  X(info_download_percent,  "info.download_percent") \
  X(info_download_error,    "info.download_error")   \
                                                     \
  X(progressbar_progress,   "progressbar.progress")  \
  X(progressbar_rest,       "progressbar.rest")      \
                                                     \
  X(tabbar_selected,        "tabbar.selected")       \
  X(tabbar_unselected,      "tabbar.unselected")     \
                                                     \
  X(list_item_even,         "list.item_even")        \
  X(list_item_odd,          "list.item_odd")         \
  X(list_item_selection,    "list.item_selection")   \
                                                     \
  X(infoline_position,      "infoline.position")     \
  X(infoline_state,         "infoline.state")        \
                                                     \
  X(help_widget_name,       "help.widget_name")      \
  X(help_key_name,          "help.key_name")         \
  X(help_command_name,      "help.command_name")     \
  X(help_command_desc,      "help.command_desc")     \

struct LoadedColors {
#define X(IDENTIFIER, _) attr_t IDENTIFIER;
  THEME_ELEMENT_IDS
#undef X
};

struct Theme {
  struct Definition {
    short fg;
    short bg;
    attr_t attributes;

    constexpr Definition(short fg_ = -2, short bg_ = -2, attr_t attrs_ = 0) noexcept
      : fg(fg_), bg(bg_), attributes(attrs_)
    {}
  };

#define X(IDENTIFIER, _) Definition IDENTIFIER;
  THEME_ELEMENT_IDS
#undef X

  Definition* get(const std::string&) noexcept;
  void load_theme(LoadedColors&)      noexcept;
};

void load_theme_by_colors(int, LoadedColors&) noexcept;
extern Theme THEME_8;
extern Theme THEME_256;
extern Theme THEME_MONO;
extern LoadedColors colors;

#endif
