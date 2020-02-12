#ifndef _PLAYINGINFO_HPP
#define _PLAYINGINFO_HPP

#include "../ui.hpp"
#include "../database.hpp"
#include "../config.hpp"

namespace Views {

class PlayingInfo : public UI::Window {
  // Slot: clicked -> player.toggle
public:
  PlayingInfo(Database&);
  void setState(int);
  void setTrack(Database::Tracks::Track);
  void setPositionAndLength(int, int);
  void draw();
  void layout(UI::Pos, UI::Size);
private:
  Database::Tracks::Track track;
  PlayingInfoFormat   *fmt_top;
  PlayingInfoFormat   *fmt_bottom;
  unsigned short      track_length;
  unsigned short      track_position;
  unsigned char       state;

  void print_formatted_strings(PlayingInfoFormat*);
  void draw_position_and_length();
  void draw_track_info();
  void draw_state();
};

} // namespace Views

#endif
