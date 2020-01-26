#include "../config.hpp"
#include "../ui.hpp"
#include "../theme.hpp"
#include "../config.hpp"
#include "../colors.hpp"
#include "../common.hpp"

#include <string>
#include <cstring>
#include <map>

const char *STOPPED_HEADING = "- Ektoplayer -";

static const char* state_to_string(int playerstate) {
  switch (playerstate) {
    case 0:  return "stopped";
    case 1:  return "paused";
    default: return "playing"; /* 2 */
  }
}

namespace Ektoplayer {
  namespace Views {
    class PlayingInfo : public UI::Window {
      private:
        std::map<std::string, std::string> track;
        PlayingInfoFormat *fmt_top;
        PlayingInfoFormat *fmt_bottom;

      public:
        // Slot: clicked -> player.toggle

        PlayingInfo();
        void setTrack(const std::map<std::string, std::string>&);
        void draw();
        void layout(int, int);
        void draw_position_and_length();
        void print_formatted_strings(PlayingInfoFormat *);
    };
  }
}

using namespace Ektoplayer;
using namespace Ektoplayer::Views;

PlayingInfo :: PlayingInfo() {
}

void PlayingInfo :: layout(int height, int width) {
  size.height = 2;
  size.width  = width;
  wresize(win, height, width);
}

void PlayingInfo :: setTrack(const std::map<std::string, std::string> &_track) {
  //return if @track == t ???
  track = _track;
  //with_lock { @track = t; want_redraw }

  if (Theme::current == 256) { /*ekto-ruby refers to ICurses.colors?*/
     fmt_top    = &Config::playinginfo_format_top_256;
     fmt_bottom = &Config::playinginfo_format_bottom_256;
  }
  else if (Theme::current == 8) {
     fmt_top    = &Config::playinginfo_format_top;
     fmt_bottom = &Config::playinginfo_format_bottom;
  }
}

void PlayingInfo :: draw() {
  // [position_and_length]       [top]               [state]
  //                            [bottom]
  werase(win);
  draw_position_and_length();

  wattrset(win, UI::Colors::get("playinginfo.state"));
  const char *state = state_to_string(2 /* TODO */);
  mvwprintw(win, 0, size.width - strlen(state) - 2, "[%s]", state);

  if (track.empty()) {
    wattrset(win, 0);
    mvwaddstr(win, 1, size.width / 2 - STRLEN(STOPPED_HEADING) / 2, STOPPED_HEADING);
  } else {
    wmove(win, 0, 0);
    print_formatted_strings(fmt_top);
    wmove(win, 1, 0);
    print_formatted_strings(fmt_bottom);
  }
}

void PlayingInfo :: print_formatted_strings(PlayingInfoFormat *format) {
  unsigned int sum = 0;
  for (const auto &fmt : *format) {
    if (! fmt.tag.empty())
      sum += track[fmt.tag].size();
    else
      sum += fmt.text.size();
  }

  wmove(win, getcury(win), size.width/2 - sum/2); // center(win, sum);
  for (const auto &fmt : *format) {
    wattrset(win, UI::Colors::set("", fmt.fg, fmt.bg, fmt.attributes));
    if (! fmt.tag.empty())
      waddstr(win, track[fmt.tag].c_str());
    else
      waddstr(win, fmt.text.c_str());
  }
}

void PlayingInfo :: draw_position_and_length() {
  int pos = 33, len = 333;
  wattrset(win, UI::Colors::get("playinginfo.position"));
  mvwprintw(win, 0, 0, "[%02d:%02d/%02d:%02d]", pos/60, pos%60, len/60, len%60);
}

#if TEST_PLAYINGINFO
#include <iostream>
#include <unistd.h>
int main() {
  initscr();
  start_color();
  use_default_colors();
  UI::Color::init();
  UI::Colors::init();
  UI::Attribute::init();
  Config::init();
  Theme::loadTheme(256);
  
  try {
    PlayingInfo p;
    p.layout(LINES, COLS);
    p.setTrack({{"title", "Title goes here"}, {"artist", "Artist"}, {"album", "Album"}, {"year", "2020"}});
    p.draw();
    p.refresh();
    pause();
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
  return 0;
}
#endif
