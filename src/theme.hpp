#ifndef THEME_HPP
#define THEME_HPP

#include CURSES_INC

#include <string>

#define THEME_HPP__FOREACH_ELEMENT(_)                \
  _(default_,               "default")               \
  _(url,                    "url")                   \
                                                     \
  _(info_head,              "info.head")             \
  _(info_tag,               "info.tag")              \
  _(info_value,             "info.value")            \
  _(info_description,       "info.description")      \
  _(info_download_file,     "info.file")             \
  _(info_download_percent,  "info.download_percent") \
  _(info_download_error,    "info.download_error")   \
                                                     \
  _(progressbar_progress,   "progressbar.progress")  \
  _(progressbar_rest,       "progressbar.rest")      \
                                                     \
  _(tabbar_selected,        "tabbar.selected")       \
  _(tabbar_unselected,      "tabbar.unselected")     \
                                                     \
  _(list_item_even,         "list.item_even")        \
  _(list_item_odd,          "list.item_odd")         \
  _(list_item_selection,    "list.item_selection")   \
                                                     \
  _(infoline_position,      "infoline.position")     \
  _(infoline_state,         "infoline.state")        \
                                                     \
  _(help_widget_name,       "help.widget_name")      \
  _(help_key_name,          "help.key_name")         \
  _(help_command_name,      "help.command_name")     \
  _(help_command_desc,      "help.command_desc")     \


struct LoadedColors {
#define THEME_HPP__DECLARE_ATTR_T(NAME, _) attr_t NAME;
  THEME_HPP__FOREACH_ELEMENT(THEME_HPP__DECLARE_ATTR_T)
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

#define THEME_HPP__DECLARE_DEFINITION(NAME, _) Definition NAME;
  THEME_HPP__FOREACH_ELEMENT(THEME_HPP__DECLARE_DEFINITION)

  Definition* get(const std::string&) noexcept;
  void load_theme(LoadedColors&)      noexcept;
};

void load_theme_by_colors(int, LoadedColors&) noexcept;
extern Theme THEME_8;
extern Theme THEME_256;
extern Theme THEME_MONO;
extern LoadedColors colors;

#endif
