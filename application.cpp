#include "ektoplayer.hpp"
#include "database.hpp"
#include "bindings.hpp"
#include "updater.hpp"
#include "player.hpp"
#include "config.hpp"
#include "colors.hpp"
#include "theme.hpp"
#include "views/mainwindow.hpp"

#include <unistd.h>  // TODO: Remove me
#include <signal.h>
#include <glob.h>

#include <libxml/xmlversion.h>

#include <boost/filesystem/operations.hpp>

#include <fstream>

namespace fs = boost::filesystem;
using namespace Ektoplayer;

static Database database;
static void init();
static void program();
static void cleanup();
static bool redraw = true;
static void on_SIGWINCH(int) { redraw = true; }
static const char* emsg;

int main(int argc, char **argv) {
  LIBXML_TEST_VERSION /* Check for ABI mismatch */
  std::string err;

  try {
    init();
    program();
  }
  catch (const std::exception &e) { err = e.what();                 }
  catch (const char *e)           { err = e;                        }
  catch (...)                     { err = "Unknown exception type"; }
  endwin();
  delwin(stdscr);

  if (err.size())
    std::cout << emsg << ": " << err << std::endl;

  return !! err.size();
}

static void init() {
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

  emsg = "Initialization error. This is a bug";
  Config::init();
  Bindings::init();

  emsg = "Error while reading configuration file";
  if (fs::exists(Ektoplayer::config_file()))
    Config::read(Ektoplayer::config_file());

  emsg = "Could not create config directory";
  fs::create_directories(Ektoplayer::config_dir());

  emsg = "Could not create cache directory";
  if (Config::use_cache)
    if (! fs::is_directory(Config::cache_dir))
      fs::create_directory(Config::cache_dir);

  emsg = "Could not create temp_dir";
  if (! fs::is_directory(Config::temp_dir))
    fs::create_directory(Config::temp_dir);

  emsg = "Could not create download_dir";
  if (! fs::is_directory(Config::download_dir))
    fs::create_directory(Config::download_dir);

  emsg = "Could not create archive_dir";
  if (! fs::is_directory(Config::archive_dir))
    fs::create_directory(Config::archive_dir);

  emsg = "Error opening log file";
  std::ios::sync_with_stdio(false);
  std::ofstream *logfile = new std::ofstream();
  logfile->exceptions(std::ofstream::failbit|std::ofstream::badbit);
  logfile->open(Config::log_file, std::ofstream::out|std::ofstream::app);
  std::cerr.rdbuf(logfile->rdbuf());

  // We know that the database will *at least* hold this amount of data,
  // so pre allocate the buffers. (Numbers from February, 2020)
#define STYLES 25
#define ALBUMS 2078
#define TRACKS 14402
  database.styles.reserve(STYLES);
  database.albums.reserve(ALBUMS);
  database.tracks.reserve(TRACKS);
  // These are the average string lengths.
  database.pool_desc.reserve(ALBUMS * 720);
  database.pool_mp3_url.reserve(ALBUMS * 39);
  database.pool_wav_url.reserve(ALBUMS * 39);
  database.pool_flac_url.reserve(ALBUMS * 39);
  database.pool_cover_url.reserve(ALBUMS * 35);
  database.pool_album_url.reserve(ALBUMS * 22);
  database.pool_track_url.reserve(TRACKS * 30);
  database.pool_style_url.reserve(STYLES * 7);
  database.pool_meta.reserve(ALBUMS * 15 + TRACKS * 25);

  emsg = "Database file corrupted? Try again, then delete it. Sorry!";
  if (fs::exists(Config::database_file))
    database.load(Config::database_file);

  // Set the error message to something more generic
  emsg = "Error";

  // All colors are beautiful
  Theme::loadTheme(Config::use_colors != -1 ? Config::use_colors : COLORS);

  atexit(cleanup);
  signal(SIGWINCH, on_SIGWINCH);
}

static void program() {
  std::cerr << "Database track count on start: " << database.tracks.size() << std::endl;

  Updater updater(database);
  if (database.tracks.size() < 42)
    updater.start(0); // Fetch all pages
  else if (Config::small_update_pages > 0)
    updater.start(-Config::small_update_pages); // Fetch last N pages

  Mpg123Player player;
  Views::MainWindow mainwindow(database);
  Actions actions(mainwindow, database, player);

  //player.play("/home/braph/.cache/ektoplayer/aerodromme-crop-circle.mp3");

  int c;
MAINLOOP:
  player.poll(); // First instruction, player needs some time to fetch info

  if (updater.write_to_database()) {
    try { database.save(Config::database_file); }
    catch (const std::exception &e) {
      throw std::runtime_error(std::string("Error saving database file: ") + e.what());
    }
  }

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
  wtimeout(win, 700);
  c = wgetch(win);
  if (c != ERR) {
    if (Bindings::global[c]) {
      c = actions.call((Actions::ActionID) Bindings::global[c]);
      if (c == Actions::QUIT)
        return;
      else if (c == Actions::REDRAW)
        redraw = true;
    }
  }

goto MAINLOOP;
}

static void cleanup() {
  glob_t globbuf;
  char pattern[8192];
  sprintf(pattern, "%s" PATH_SEP "~ekto-*", Config::temp_dir.c_str());
  glob(pattern, GLOB_NOSORT|GLOB_NOESCAPE, NULL, &globbuf);
  while (globbuf.gl_pathc--)
    unlink(globbuf.gl_pathv[globbuf.gl_pathc]);
#if PEDANTIC_FREE
  globfree(&globbuf);
#endif
}

#if 0
TODO config::audio_system
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
