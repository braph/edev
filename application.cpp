#include "ektoplayer.hpp"
#include "database.hpp"
#include "updater.hpp"
#include "player.hpp"
#include "config.hpp"
#include "colors.hpp"
#include "theme.hpp"
#include "xml.hpp"
#include "views/mainwindow.hpp"

#include <unistd.h>  // TODO: Remove me
#include <signal.h>
#include <glob.h>

#include <fstream>
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;
using namespace Ektoplayer;

static Database database;
static void init();
static void program();
static void cleanup();
static bool hv_SIGWINCH = true;
static void on_SIGWINCH(int) { hv_SIGWINCH = true; }
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

  Config::database_file = "/tmp/ekto-test.db"; // XXX TODO!
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

  player.play("/home/braph/.cache/ektoplayer/aerodromme-crop-circle.mp3");

  int c;
MAINLOOP:
  player.poll(); // First instruction, player needs some time to fetch info

  if (updater.write_to_database()) {
    try { database.save(Config::database_file); }
    catch (const std::exception &e) {
      throw std::runtime_error(std::string("Error saving database file: ") + e.what());
    }
  }

  if (hv_SIGWINCH) {
    hv_SIGWINCH = false;
    mainwindow.layout({0,0}, {LINES,COLS});
  }

  mainwindow.progressBar.setPercent(player.percent());
  mainwindow.playingInfo.setPositionAndLength(player.position(), player.length());
  mainwindow.playingInfo.setState();

  mainwindow.draw();
  mainwindow.noutrefresh();
  doupdate();

  WINDOW *win = mainwindow.active_win();
  wtimeout(win, 700);
  c = wgetch(win);
  switch (c) {
    case 'q': return;
    case 'b': player.seek_backward(10); break;
    case 'f': player.seek_forward(10); break;
    case 'x': player.poll(); break;
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
