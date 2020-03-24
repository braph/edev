#ifndef VIEWS_PLAYINGINFO_HPP
#define VIEWS_PLAYINGINFO_HPP

#include "../ui.hpp"
#include "../database.hpp" // XXX get rid of this
#include "../context.hpp"
#include "../player.hpp"
#include <vector>
#include <string>

namespace Views {

struct InfoLineFormatString {
  short fg;
  short bg;
  unsigned int attributes;
  Database::ColumnID tag;
  std::string text;

  InfoLineFormatString()
    : fg(-1), bg(-1), attributes(0)
  {
  }
};

typedef std::vector<InfoLineFormatString> InfoLineFormat;

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
