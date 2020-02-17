#include "ektoplayer.hpp"
#include "trackloader.hpp"
#include "downloads.hpp"
#include "database.hpp"
#include "bindings.hpp"
#include "updater.hpp"
#include "player.hpp"
#include "config.hpp"
#include "colors.hpp"
#include "theme.hpp"
#include "views/mainwindow.hpp"

#include <unistd.h>
#include <signal.h>
#include <glob.h>

#include <libxml/xmlversion.h>

#include <boost/filesystem/operations.hpp>

#include <fstream>

using namespace Ektoplayer;
namespace fs = boost::filesystem;

static bool redraw = true;
static void on_SIGWINCH(int) { redraw = true; }

class Application {
public:
  Application();
 ~Application();
  void run();
  void init();
  void cleanup();
private:
  Database database;
  Downloads downloads;
  Updater updater;
  TrackLoader trackloader;
  Mpg123Player player;
  const char* error;
};

Application :: Application()
: database()
, downloads(10)
, updater(database, downloads)
, trackloader(downloads)
, player()
{
  try { init(); }
  catch (const std::exception &e) {
    throw std::runtime_error(std::string(error) + ": " + e.what());
  }
}

Application :: ~Application() {
  cleanup();

  try {
    database.shrink_to_fit();
    database.save(Config::database_file);
  }
  catch (const std::exception &e) {
    throw std::runtime_error(std::string("Error saving database file: ") + e.what());
  }
}

void Application :: init() {
  // Set terminal title
  std::cout << "\033]0;ektoplayer\007" << std::endl;

  // Initialize curses
  initscr();
  cbreak();
  noecho();
  start_color();
  use_default_colors();
  curs_set(0);
  mousemask(ALL_MOUSE_EVENTS/*|REPORT_MOUSE_POSITION*/, NULL);

  error = "Initialization error. This is a bug";
  Config::init();
  Bindings::init();

  error = "Error while reading configuration file";
  if (fs::exists(Ektoplayer::config_file()))
    Config::read(Ektoplayer::config_file());

  error = "Could not create config directory";
  fs::create_directories(Ektoplayer::config_dir());

  error = "Could not create cache directory";
  if (Config::use_cache)
    if (! fs::is_directory(Config::cache_dir))
      fs::create_directory(Config::cache_dir);

  error = "Could not create temp_dir";
  if (! fs::is_directory(Config::temp_dir))
    fs::create_directory(Config::temp_dir);

  error = "Could not create download_dir";
  if (! fs::is_directory(Config::download_dir))
    fs::create_directory(Config::download_dir);

  error = "Could not create archive_dir";
  if (! fs::is_directory(Config::archive_dir))
    fs::create_directory(Config::archive_dir);

  error = "Error opening log file";
  std::ios::sync_with_stdio(false);
  std::ofstream *logfile = new std::ofstream();
  logfile->exceptions(std::ofstream::failbit|std::ofstream::badbit);
  logfile->open(Config::log_file, std::ofstream::out|std::ofstream::app);
  std::cerr.rdbuf(logfile->rdbuf());

  // The database will *at least* hold this amount of data
  database.styles.reserve(EKTOPLAZM_STYLE_COUNT);
  database.albums.reserve(EKTOPLAZM_ALBUM_COUNT);
  database.tracks.reserve(EKTOPLAZM_TRACK_COUNT);
  database.pool_desc.reserve(EKTOPLAZM_DESC_SIZE);
  database.pool_archive_url.reserve(EKTOPLAZM_ARCHIVE_URL_SIZE);
  database.pool_cover_url.reserve(EKTOPLAZM_COVER_URL_SIZE);
  database.pool_album_url.reserve(EKTOPLAZM_ALBUM_URL_SIZE);
  database.pool_track_url.reserve(EKTOPLAZM_TRACK_URL_SIZE);
  database.pool_style_url.reserve(EKTOPLAZM_STYLE_URL_SIZE);
  database.pool_meta.reserve(EKTOPLAZM_META_SIZE);

  error = "Database file corrupted? Try again, then delete it. Sorry!";
  if (fs::exists(Config::database_file))
    database.load(Config::database_file);

  // Set the error message to something more generic
  error = "Error";

  // All colors are beautiful
  Theme::loadTheme(Config::use_colors != -1 ? Config::use_colors : COLORS);
}

void Application :: run() {
  std::cerr << "Database track count on start: " << database.tracks.size() << std::endl;

  if (database.tracks.size() < 42)
    updater.start(0); // Fetch all pages
  else if (Config::small_update_pages > 0)
    updater.start(-Config::small_update_pages); // Fetch last N pages

  //player.audio_system = Config::audio_system;
  Views::MainWindow mainwindow(database);
  Actions actions(mainwindow, database, player);

  //player.play("/home/braph/.cache/ektoplayer/aerodromme-crop-circle.mp3");

  int c;
  int n = 4;
  int downloading;
MAINLOOP:
  // First instruction, player needs some time to fetch info
  player.poll();
  // Do at max 10 download iterations
  for (downloading = 0; downloading < 10 && downloads.work(); ++downloading);

  if (redraw) {
    redraw = false;
    mainwindow.layout({0,0}, {LINES,COLS});
    mainwindow.draw();
  }

  mainwindow.progressBar.setPercent(player.percent());
  mainwindow.playingInfo.setPositionAndLength(player.position(), player.length());
  mainwindow.playingInfo.setState(player.getState());

  mainwindow.noutrefresh();
  doupdate();

  WINDOW *win = mainwindow.active_win();
  wtimeout(win, 1000);
  c = wgetch(win);
  if (c != ERR) {
    if (c == 'p')
      player.play(trackloader.getFileForTrack(database.tracks[n], false));
    else if (c == '>')
      ++n;
    //else if (c == 'l') {
    //  trackloader.getFileForTrack(database.tracks[n], false);
    //}
    else if (c != ERR) {
      if (Bindings::global[c])
        c = Bindings::global[c];
      else if (Bindings::playlist[c])
        c = Bindings::playlist[c];

      if (c) {
        c = actions.call(static_cast<Actions::ActionID>(c));
        if (c == Actions::QUIT)
          return;
        else if (c == Actions::REDRAW)
          redraw = true;
      }
    }
  }

goto MAINLOOP;
}

void Application :: cleanup() {
  glob_t globbuf;
  char pattern[8192];
  sprintf(pattern, "%s" PATH_SEP "~ekto-*", Config::temp_dir.c_str());
  glob(pattern, GLOB_NOSORT|GLOB_NOESCAPE, NULL, &globbuf);
  for (size_t i = 0; i < globbuf.gl_pathc; ++i)
    unlink(globbuf.gl_pathv[i]);
#if PEDANTIC_FREE
  globfree(&globbuf);
#endif
}

int main() {
  LIBXML_TEST_VERSION /* Check for ABI mismatch */
  signal(SIGWINCH, on_SIGWINCH);

  try {
    Application app;
    app.run();
  }
  catch (const std::exception &e) {
    endwin();
    std::cout << e.what() << std::endl;
    return 1;
  }

  delwin(stdscr);
  return 0;
}

#if 0
TODO database.events.on(:update_finished, &browser.method(:reload))
TODO preload playlist

  player.events.on(:stop) do |reason|
   operations.send(:'playlist.play_next') if reason == :track_completed
  end

  time_t now = time(NULL);
  if (Config::prefetch && now % 5 == 0) {
    if (player.state != PLAYING)
      continue;

     current_download_track = nil
     loop do
        sleep 5 // clock() % 5?

        next_track = playlist.get_next_pos
        next if current_download_track == next_track

        if player.length > 30 and player.position_percent > 0.5
           trackloader.get_track_file(playlist[next_track]['url'])
           current_download_track = next_track
           sleep 5
        end
     end
#endif
