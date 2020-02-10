#ifndef _PLAYINGINFO_HPP
#define _PLAYINGINFO_HPP

#include "../ui.hpp"
#include "../database.hpp"
#include "../config.hpp"

namespace Views {
  class PlayingInfo : public UI::Window {
  private:
    int track_length;
    int track_position;
    Database::Tracks::Track track;
    PlayingInfoFormat *fmt_top;
    PlayingInfoFormat *fmt_bottom;
    void print_formatted_strings(PlayingInfoFormat *);
    void draw_position_and_length();
    void draw_track_info();
    void draw_state();

  public:
    // Slot: clicked -> player.toggle

    PlayingInfo(Database&);
    void setTrack(Database::Tracks::Track);
    void setPositionAndLength(int, int);
    void draw();
    void layout(UI::Pos, UI::Size);
  };
}

#endif
