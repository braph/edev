#ifndef _PLAYINGINFO_HPP
#define _PLAYINGINFO_HPP

#include "../ui.hpp"
#include "../database.hpp"
#include <vector>
#include <string>

namespace Views {

struct PlayingInfoFormatFoo {
  short fg;
  short bg;
  unsigned int attributes;
  Database::ColumnID tag;
  std::string text;

  PlayingInfoFormatFoo()
    : fg(-1), bg(-1), attributes(0)
  {
  }
};

typedef std::vector<PlayingInfoFormatFoo> PlayingInfoFormat;

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
