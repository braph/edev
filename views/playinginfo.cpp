#include "playinginfo.hpp"

#include "../config.hpp"
#include "../theme.hpp"
#include "../colors.hpp"
#include "../common.hpp"

#include <cstring>

#define STOPPED_HEADING "- Ektoplayer -"
#define STATE_LEN 9
static const char state_to_string[4][STATE_LEN+1] = {
  /* 0 */ "[stopped]",
  /* 1 */ " [paused]",
  /* 2 */ "[playing]",
  /* 3 */ "[loading]"
};

using namespace UI;
using namespace Views;

PlayingInfo :: PlayingInfo(Database::Database &db)
: track_length(0), track_position(0), state(0)
{
  if (Theme::current == 256) {
     fmt_top    = &Config::playinginfo_format_top_256;
     fmt_bottom = &Config::playinginfo_format_bottom_256;
  }
  else if (Theme::current == 8) {
     fmt_top    = &Config::playinginfo_format_top;
     fmt_bottom = &Config::playinginfo_format_bottom;
  }
  draw();
}

void PlayingInfo :: setTrack(Database::Tracks::Track track) {
  if (track != this->track) {
    this->track = track;
    draw(); // we need werase()
  }
}

void PlayingInfo :: setState(int state) {
  if (state != this->state) {
    this->state = state;
    draw_state();
  }
}

void PlayingInfo :: setPositionAndLength(int position, int length) {
  if (position != this->track_position || length != this->track_length) {
    track_position = position;
    track_length   = length;
    draw_position_and_length();
  }
}

void PlayingInfo :: layout(Pos pos, Size size) {
  size.height = 2;
  if (size != this->size) {
    this->size = size;
    wresize(win, size.height, size.width);
  }
  if (pos != this->pos) {
    this->pos = pos;
    mvwin(win, pos.y, pos.x);
  }
}

void PlayingInfo :: draw_state() {
  attrSet(Theme::get(Theme::PLAYINGINFO_STATE));
  mvAddStr(0, size.width - STATE_LEN, state_to_string[state]);
}

void PlayingInfo :: draw_position_and_length() {
  attrSet(Theme::get(Theme::PLAYINGINFO_POSITION));
  mvPrintW(0, 0, "[%02d:%02d/%02d:%02d]",
      track_position/60, track_position%60,
      track_length/60, track_length%60);
}

void PlayingInfo :: draw_track_info() {
  if (! track) {
    attrSet(0);
    mvAddStr(1, size.width / 2 - int(STRLEN(STOPPED_HEADING) / 2), STOPPED_HEADING);
  } else {
    moveCursor(0, 0); print_formatted_strings(*fmt_top);
    moveCursor(1, 0); print_formatted_strings(*fmt_bottom);
  }
}

void PlayingInfo :: draw() {
  erase();
  draw_position_and_length();
  draw_track_info();
  draw_state();
}

#include "rm_trackstr.cpp"

void PlayingInfo :: print_formatted_strings(const PlayingInfoFormat& format) {
  size_t sum = 0;

  for (const auto& fmt : format) {
    size_t len;
    if (fmt.text.length())
      len = mbstowcs(NULL, fmt.text.c_str(), 0);
    else
      len = mbstowcs(NULL, trackField(track, fmt.tag), 0);
    sum += len; // TODO: Error handling...
  }

  moveCursor(getcury(win), size.width/2 - int(sum/2));
  for (const auto& fmt : format) {
    attrSet(UI::Colors::set(fmt.fg, fmt.bg, fmt.attributes));
    if (fmt.text.length())
      *this << toWideString(fmt.text);
    else
      *this << toWideString(trackField(track, fmt.tag));
  }
}

#ifdef TEST_PLAYINGINFO
#include "../test.hpp"
int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Config::init();
  Theme::loadTheme(COLORS);
  Database db;
  db.load(TEST_DB);
  
  PlayingInfo p(db);
  p.layout({0,0}, {LINES, COLS});
  for (size_t i = 0; i < 100; ++i) {
    p.setTrack(db.tracks[i]);

    for (int pos = 0; pos < 300; pos+=100) {
      p.setPositionAndLength(pos, 300);
      p.noutrefresh();
      doupdate();
      usleep(1000 * 40);
    }
  }

  TEST_END();
}
#endif
