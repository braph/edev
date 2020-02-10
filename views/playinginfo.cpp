#include "../config.hpp"
#include "../ui.hpp"
#include "../theme.hpp"
#include "../config.hpp"
#include "../colors.hpp"
#include "../common.hpp"
#include "../database.hpp"

#include <string>
#include <cstring>

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
        Database::Tracks::Track track;
        PlayingInfoFormat *fmt_top;
        PlayingInfoFormat *fmt_bottom;

      public:
        // Slot: clicked -> player.toggle

        PlayingInfo(Database&);
        void setTrack(Database::Tracks::Track);
        void draw();
        void layout(int, int);
        void draw_position_and_length();
        void print_formatted_strings(PlayingInfoFormat *);
    };
  }
}

using namespace Ektoplayer;
using namespace Ektoplayer::Views;

PlayingInfo :: PlayingInfo(Database &db)
: track(db, 0)
{
}

void PlayingInfo :: layout(int height, int width) {
  size.height = 2;
  size.width  = width;
  wresize(win, height, width);
}

void PlayingInfo :: setTrack(Database::Tracks::Track _track) {
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

  wattrset(win, Theme::get(Theme::PLAYINGINFO_STATE));
  const char *state = state_to_string(2 /* TODO */);
  mvwprintw(win, 0, size.width - strlen(state) - 2, "[%s]", state);

  if (! track) {
    wattrset(win, 0);
    mvwaddstr(win, 1, size.width / 2 - STRLEN(STOPPED_HEADING) / 2, STOPPED_HEADING);
  } else {
    wmove(win, 0, 0);
    print_formatted_strings(fmt_top);
    wmove(win, 1, 0);
    print_formatted_strings(fmt_bottom);
  }
}

static const char* trackField(Database::Tracks::Track track, Database::ColumnID id) {
#define TOS(INT, FMT) ffff
#define DB Database
  static char buf[32];
  switch (id) {
    case DB::STYLE_NAME:            return "TODO";
    case DB::ALBUM_TITLE:           return track.album().title();
    case DB::ALBUM_ARTIST:          return track.album().artist();
    case DB::ALBUM_DESCRIPTION:     return track.album().description();
    case DB::ALBUM_DATE:            return "TODO";
    case DB::ALBUM_RATING:          return "TODO";
    case DB::ALBUM_VOTES:           return "TODO";
    case DB::ALBUM_DOWNLOAD_COUNT:  return "TODO";
    //case TRACK_NUMBER: // TODO
    //case TRACK_NUMBER: // TODO
    //case TRACK_NUMBER: // TODO
    case DB::TRACK_TITLE:           return track.title();
    case DB::TRACK_ARTIST:          return track.artist();
    case DB::TRACK_REMIX:           return track.remix();
    case DB::TRACK_NUMBER:          return "TODO";
    case DB::TRACK_BPM:             return "TODO";
  }
}

void PlayingInfo :: print_formatted_strings(PlayingInfoFormat *format) {
  unsigned int sum = 0;
  for (const auto &fmt : *format) {
    if (! fmt.text.empty())
      sum += fmt.text.size();
    else
      sum += strlen(trackField(track, fmt.tag));
  }

  wmove(win, getcury(win), size.width/2 - sum/2); // center(win, sum);
  for (const auto &fmt : *format) {
    wattrset(win, UI::Colors::set(fmt.fg, fmt.bg, fmt.attributes));
    if (! fmt.text.empty())
      waddstr(win, fmt.text.c_str());
    else
      waddstr(win, trackField(track, fmt.tag));
  }
}

void PlayingInfo :: draw_position_and_length() {
  int pos = 33, len = 333;
  wattrset(win, Theme::get(Theme::PLAYINGINFO_POSITION));
  mvwprintw(win, 0, 0, "[%02d:%02d/%02d:%02d]", pos/60, pos%60, len/60, len%60);
}

#if TEST_PLAYINGINFO
#include "../test.hpp"
int main() {
  initscr();
  start_color();
  use_default_colors();
  Config::init();
  Theme::loadTheme(256);
  Database db;
  db.load(TEST_DB);
  
  try {
    PlayingInfo p(db);
    p.layout(LINES, COLS);
    for (int i = 0; i < db.tracks.size(); ++i) {
      p.setTrack(db.tracks[i]);
      p.draw();
      p.refresh();
      sleep(1);
    }
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
  return 0;
}
#endif
