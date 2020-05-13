#ifndef VIEWS_PLAYINGINFO_HPP
#define VIEWS_PLAYINGINFO_HPP

#include "../ui.hpp"
#include "../database.hpp" // XXX get rid of this
#include "../application.hpp"
#include "../player.hpp"
#include "../lib/staticvector.hpp"
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
    unsigned int attributes_ = 0)
  : fg(fg_)
  , bg(bg_)
  , attributes(attributes_)
  , text(text_)
  , tag(tag_)
  {}
};

using InfoLineFormat = StaticVector<InfoLineFormatString, 10>;

class InfoLine : public UI::Window {
  // XXX Slot: clicked -> player.toggle
public:
  InfoLine();
  void setState(Mpg123Player::State);
  void setTrack(Database::Tracks::Track);
  void setPositionAndLength(int, int);
  void draw();
  void layout(UI::Pos, UI::Size);

private:
  Database::Tracks::Track track;
  InfoLineFormat*         fmt_top;
  InfoLineFormat*         fmt_bottom;
  int                     track_length;
  int                     track_position;
  Mpg123Player::State     state;

  void print_formatted_strings(int, const InfoLineFormat&);
  void draw_position_and_length();
  void draw_track_info();
  void draw_state();
};

} // namespace Views

#endif
