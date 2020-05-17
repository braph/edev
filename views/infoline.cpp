#include "infoline.hpp"

#include "../config.hpp"
#include "../theme.hpp"
#include "../ui/colors.hpp"
#include "rm_trackstr.cpp" // XXX

#include <cstring>

#define STOPPED_HEADING "- Ektoplayer -"
#define STOPPED_HEADING_LEN (sizeof(STOPPED_HEADING) - 1)

#define STATE_LEN 9
static const char state_to_string[4][STATE_LEN+1] = {
  /* 0 */ "[stopped]",
  /* 1 */ " [paused]",
  /* 2 */ "[playing]",
  /* 3 */ "[loading]"
};

using namespace UI;
using namespace Views;
using ElementID = Theme::ElementID;

InfoLine :: InfoLine()
: UI::Window({0,0}, {2,0})
, track_length(0)
, track_position(0)
, state(Mpg123Player::STOPPED)
{
  switch (Theme::current) {
  case Theme::ThemeID::THEME_256:
    fmt_top     = &Config::infoline_format_top_256;
    fmt_bottom  = &Config::infoline_format_bottom_256;
    break;
  case Theme::ThemeID::THEME_8:
     fmt_top    = &Config::infoline_format_top;
     fmt_bottom = &Config::infoline_format_bottom;
     break;
  case Theme::ThemeID::THEME_MONO: // TODO: infoline_format_top_mono
     fmt_top    = &Config::infoline_format_top;
     fmt_bottom = &Config::infoline_format_bottom;
  }
  draw();
}

void InfoLine :: setTrack(Database::Tracks::Track track) noexcept {
  if (track != this->track) {
    this->track = track;
    draw(); // we need werase()
  }
}

void InfoLine :: setState(Mpg123Player::State state) noexcept {
  if (state != this->state) {
    this->state = state;
    draw_state();
  }
}

void InfoLine :: setPositionAndLength(int position, int length) noexcept {
  if (position != this->track_position || length != this->track_length) {
    track_position = position;
    track_length   = length;
    draw_position_and_length();
  }
}

void InfoLine :: layout(Pos pos, Size size) {
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

void InfoLine :: draw_state() {
  attrSet(Theme::get(ElementID::INFOLINE_STATE));
  mvAddStr(0, size.width - STATE_LEN, state_to_string[state]);
}

void InfoLine :: draw_position_and_length() {
  attrSet(Theme::get(ElementID::INFOLINE_POSITION));
  mvPrintW(0, 0, "[%02d:%02d/%02d:%02d]", track_position/60, track_position%60, track_length/60, track_length%60);
}

void InfoLine :: draw_track_info() {
  if (! track) {
    attrSet(0);
    mvAddStr(1, size.width / 2 - int(STOPPED_HEADING_LEN / 2), STOPPED_HEADING);
  } else {
    print_formatted_strings(0, *fmt_top);
    print_formatted_strings(1, *fmt_bottom);
  }
}

void InfoLine :: draw() {
  erase();
  draw_position_and_length();
  draw_track_info();
  draw_state();
}

void InfoLine :: print_formatted_strings(int y, const InfoLineFormat& format) {
  size_t sum = 0;

  for (const auto& fmt : format) {
    size_t len;
    if (fmt.text.length())
      len = std::mbstowcs(NULL, fmt.text.c_str(), 0);
    else
      len = std::mbstowcs(NULL, trackField(track, fmt.tag), 0);
    sum += len; // TODO: Error handling of mbstowcs
  }

  moveCursor(y, size.width / 2 - int(sum / 2));
  for (const auto& fmt : format) {
    attrSet(UI::Colors::set(fmt.fg, fmt.bg, fmt.attributes));
    if (fmt.text.length())
      *this << toWideString(fmt.text);
    else
      *this << toWideString(trackField(track, fmt.tag));
  }
}

#ifdef TEST_INFOLINE
#include "../lib/test.hpp"
int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Config::init();
  Theme::loadTheme(COLORS);
  Database db;
  db.load(TEST_DB);
  
  InfoLine p(db);
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