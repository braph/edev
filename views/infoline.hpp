#ifndef VIEWS_PLAYINGINFO_HPP
#define VIEWS_PLAYINGINFO_HPP

#include "../ui.hpp"
#include "../database.hpp"
#include <vector>
#include <string>

namespace Views {

struct InfoLineFormatFoo {
  short fg;
  short bg;
  unsigned int attributes;
  Database::ColumnID tag;
  std::string text;

  InfoLineFormatFoo()
    : fg(-1), bg(-1), attributes(0)
  {
  }
};

typedef std::vector<InfoLineFormatFoo> InfoLineFormat;

class InfoLine : public UI::Window {
  // Slot: clicked -> player.toggle
public:
  InfoLine(Database::Database&);
  void setState(int);
  void setTrack(Database::Tracks::Track);
  void setPositionAndLength(int, int);
  void draw();
  void layout(UI::Pos, UI::Size);
private:
  Database::Tracks::Track track;
  InfoLineFormat   *fmt_top;
  InfoLineFormat   *fmt_bottom;
  unsigned short      track_length;
  unsigned short      track_position;
  unsigned char       state;

  void print_formatted_strings(const InfoLineFormat&);
  void draw_position_and_length();
  void draw_track_info();
  void draw_state();
};

} // namespace Views

#endif
