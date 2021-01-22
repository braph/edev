#ifndef VIEWS_PLAYINGINFO_HPP
#define VIEWS_PLAYINGINFO_HPP

#include "../application.hpp"
#include "../database.hpp"
#include "../mpg123playback.hpp"
#include "../ui/core.hpp"

#include <lib/staticvector.hpp>

#include <string>
#include <functional>

namespace Views {

struct InfoLineFormatString {
  short fg;
  short bg;
  attr_t attributes;
  std::string text;
  Database::ColumnID tag;

  InfoLineFormatString(
    Database::ColumnID tag_ = Database::COLUMN_NONE,
    std::string text_ = "",
    short fg_ = -1,
    short bg_ = -1,
    attr_t attributes_ = 0
  )
    : fg(fg_)
    , bg(bg_)
    , attributes(attributes_)
    , text(std::move(text_))
    , tag(tag_)
  {}
};

using InfoLineFormat = StaticVector<InfoLineFormatString, 10>;

class InfoLine : public UI::Window {
public:
  InfoLine();

  void draw()                             override;
  void layout(UI::Pos, UI::Size)          override;
  bool handle_mouse(MEVENT&)              override;

  void state(Mpg123Playback::State)       noexcept;
  void track(Database::Tracks::Track)     noexcept;
  void set_position_and_length(int, int)  noexcept;

  std::function<void()> on_click;

private:
  Database::Tracks::Track _track;
  InfoLineFormat*         _fmt_top;
  InfoLineFormat*         _fmt_bottom;
  int                     _track_length;
  int                     _track_position;
  Mpg123Playback::State   _state;

  void print_formatted_strings(int, const InfoLineFormat&);
  void draw_position_and_length();
  void draw_track_info();
  void draw_state();
};

} // namespace Views

#endif
