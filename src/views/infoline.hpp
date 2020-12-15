#ifndef VIEWS_PLAYINGINFO_HPP
#define VIEWS_PLAYINGINFO_HPP

#include "../application.hpp"
#include "../database.hpp"
#include "../player.hpp"
#include "../ui/core.hpp"

#include <lib/staticvector.hpp>

#include <string>

namespace Views {

struct InfoLineFormatString {
  short fg;
  short bg;
  unsigned int attributes;
  std::string text;
  Database::ColumnID tag;

  InfoLineFormatString(
    Database::ColumnID tag_ = Database::COLUMN_NONE,
    std::string text_ = "",
    short fg_ = -1,
    short bg_ = -1,
    unsigned int attributes_ = 0
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
  // TODO Slot: clicked -> player.toggle
public:
  InfoLine();

  void draw()                             override;
  void layout(UI::Pos, UI::Size)          override;

  void state(Mpg123Player::State)         noexcept;
  void track(Database::Tracks::Track)     noexcept;
  void set_position_and_length(int, int)  noexcept;

private:
  Database::Tracks::Track _track;
  InfoLineFormat*         _fmt_top;
  InfoLineFormat*         _fmt_bottom;
  int                     _track_length;
  int                     _track_position;
  Mpg123Player::State     _state;

  void print_formatted_strings(int, const InfoLineFormat&);
  void draw_position_and_length();
  void draw_track_info();
  void draw_state();
};

} // namespace Views

#endif
