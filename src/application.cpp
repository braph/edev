#include "application.hpp"

#include "ektoplayer.hpp"
#include "trackloader.hpp"
#include "database.hpp"
#include "bindings.hpp"
#include "updater.hpp"
#include "player.hpp"
#include "config.hpp"
#include "theme.hpp"
#include "views/mainwindow.hpp"

#include <lib/cstring.hpp>

#include <libxml/xmlversion.h>

#include <clocale>
#include <csignal>
#include <type_traits>

namespace fs = Filesystem;

Context ctxt;

static volatile int caught_signal;
static void on_signal(int sig) { caught_signal = sig; }

class Application {
public:
  Application();
 ~Application();
  void init();
  void run();

private:
  Database::Database  database;
  Updater             updater;
  TrackLoader         trackloader;
  Mpg123Player        player;

  void print_db_stats();
  void cleanup_files();
};

Application :: Application()
: database()
, updater(database)
, trackloader()
, player()
{
  ctxt.player      = &player;
  ctxt.database    = &database;
  ctxt.trackloader = &trackloader;
  ctxt.updater     = &updater;
  init();
}

Application :: ~Application() {
  ::endwin();
  cleanup_files();

  try {
    // Write unoptimized database just in case shrink() fails
    database.save(Config::database_file);
    database.shrink_to_fit();
    database.save(Config::database_file);
  } catch (std::exception& e) {
    std::printf("Error saving database to file: %s\n", e.what());
  }

  log_write("Terminated gracefully.\n");
}

void Application :: init() {
  const char* e = REPORT_BUG;

  try {
    // Set terminal title
    std::printf("\033]0;ektoplayer\007" // *xterm
                "\033kektoplayer\033\\" // screen/tmux
                "\r\n");
   
    // Use the locale from the environment
    std::setlocale(LC_ALL, "");

    // Initialize curses
    ::initscr();
    ::cbreak();
    ::noecho();
    ::start_color();
    ::use_default_colors();
    ::curs_set(0);
    ::mousemask(ALL_MOUSE_EVENTS, NULL);
    ::wresize(stdscr, 1, 1); // Save some bytes...

    Bindings::init();
    Config::init();
    updater.downloads().parallel(10);

    e = "Error while reading configuration file";
    if (fs::exists(Ektoplayer::config_file()))
      Config::read(Ektoplayer::config_file().string());

    e = "Could not create config directory";
    fs::create_directories(Ektoplayer::config_dir());

    e = "Could not create cache directory";
    if (Config::use_cache)
      if (! fs::is_directory(Config::cache_dir))
        fs::create_directory(Config::cache_dir);

    e = "Could not create temp_dir";
    if (! fs::is_directory(Config::temp_dir))
      fs::create_directory(Config::temp_dir);

    e = "Could not create download_dir";
    if (! fs::is_directory(Config::download_dir))
      fs::create_directory(Config::download_dir);

    e = "Could not create archive_dir";
    if (! fs::is_directory(Config::archive_dir))
      fs::create_directory(Config::archive_dir);

    e = "Error opening log file";
    if (! std::freopen(Config::log_file.c_str(), "a", stderr))
      throw std::runtime_error(std::strerror(errno));
    std::setvbuf(stderr, NULL, _IOLBF, 0);

    e = "Error reading database file. Try again, then delete it. Sorry!";
    if (fs::exists(Config::database_file))
      database.load(Config::database_file);
    else {
      // The database will *at least* hold this amount of data
      database.styles.reserve(EKTOPLAZM_STYLE_COUNT);
      database.albums.reserve(EKTOPLAZM_ALBUM_COUNT);
      database.tracks.reserve(EKTOPLAZM_TRACK_COUNT);
      database.chunk_meta.reserve(EKTOPLAZM_META_SIZE);
      database.chunk_desc.reserve(EKTOPLAZM_DESC_SIZE);
      database.chunk_cover_url.reserve(EKTOPLAZM_COVER_URL_SIZE);
      database.chunk_album_url.reserve(EKTOPLAZM_ALBUM_URL_SIZE);
      database.chunk_track_url.reserve(EKTOPLAZM_TRACK_URL_SIZE);
      database.chunk_style_url.reserve(EKTOPLAZM_STYLE_URL_SIZE);
      database.chunk_archive_url.reserve(EKTOPLAZM_ARCHIVE_URL_SIZE);
    }

    // All colors are beautiful
    Theme::load_theme_by_colors(Config::use_colors != -1 ? Config::use_colors : COLORS);
  }
  catch (const std::exception &ex) {
    throw std::runtime_error(std::string(e) + ": " + ex.what());
  }
}

void Application :: run() {
  print_db_stats();

  if (database.tracks.size() < 1000)
    updater.start();
  else if (Config::small_update_pages > 0)
    updater.start(Config::small_update_pages);

  Views::MainWindow mainwindow;
  ctxt.mainwindow = &mainwindow;

  // Connecting widgets events
  mainwindow.progressBar.percent_changed = [&](float percent) {
    player.percent(percent);
    mainwindow.progressBar.percent(percent);
  };

  mainwindow.tabBar.index_changed = [&](int index) {
    mainwindow.tabBar.current_index(index);
    mainwindow.windows.current_index(index);
  };

  int key;
  WINDOW *win;
  MEVENT mouse;
  Database::Tracks::Track prefetching_track;

  mainwindow.playlist.playlist = database.get_tracks();

WINDOW_RESIZE:
  mainwindow.layout({0,0}, {LINES,COLS});
  mainwindow.draw();

MAINLOOP:
  switch (caught_signal) {
    case SIGINT:
    case SIGTERM:
      return;
    case SIGWINCH:
      caught_signal = 0;
      goto WINDOW_RESIZE;
  }

  player.work();

  // Song prefetching
  if (Config::prefetch > 0.0
      && player.percent() >= Config::prefetch
      && player.is_playing()
      && player.length() >= 30
      && mainwindow.playlist.container_size() >= 2)
  {
    auto list = mainwindow.playlist.list();
    auto next_track = (*list)[size_t(mainwindow.playlist.active_index() + 1) % list->size()];
    if (next_track != prefetching_track) {
      trackloader.get_file_for_track(next_track);
      prefetching_track = next_track;
    }
  }

  if (player.is_track_completed())
    Actions::call(Actions::PLAYLIST_NEXT);

  mainwindow.progressBar.percent(player.percent());
  mainwindow.infoLine.set_position_and_length(player.position(), player.length());
  mainwindow.infoLine.state(player.state());
  if (!mainwindow.playlist.empty() && mainwindow.playlist.active_index() >= 0) {
    mainwindow.infoLine.track(mainwindow.playlist.active_item());
    mainwindow.info.track(mainwindow.playlist.active_item());
  }
  mainwindow.noutrefresh();
  doupdate();

  win = mainwindow.getWINDOW();

  // Do as much download work as possible, be only interrupted by the user
  wtimeout(win, 0);
  while (trackloader.downloads().work() || updater.downloads().work()) {
    if (((key = wgetch(win)) != ERR))
      goto HANDLE_KEY;
  }

  if (trackloader.downloads().running_downloads() || trackloader.downloads().queued_downloads()
      ||  updater.downloads().running_downloads() || updater.downloads().queued_downloads())
    wtimeout(win, 100); // Short timeout, want to continue downloading soon
  else if (player.is_stopped() || player.is_paused())
    wtimeout(win, -1);  // We have *nothing* to do, wait until user hits a key
  else
    wtimeout(win, 900); // In playing state we need some UI refreshes

  key = wgetch(win);
HANDLE_KEY:
  switch (key) {
    case ERR:
      break;
    case KEY_MOUSE:
      if (OK == getmouse(&mouse))
        mainwindow.handle_mouse(mouse);
      break;
    default:
      mainwindow.handle_key(key);
  }

  goto MAINLOOP;
}

void Application :: cleanup_files() {
  Filesystem::error_code e;
  for (const auto& f : Filesystem::directory_iterator(Config::temp_dir, e))
    if (strprefix(f.path().filename().c_str(), EKTOPLAZM_TEMP_FILE_PREFIX))
      Filesystem::remove(f, e);
}

void Application :: print_db_stats() {
  log_write("Database statistics:\n");
  for (const auto table : database.tables) {
    log_write("%s (%zu): ", table->name, table->size());
#if DATABASE_USE_PACKED_VECTOR
    for (const auto column : table->columns)
      log_write("%d|", column->bits());
#endif
    log_write("\n");
  }
}

int main() {
  LIBXML_TEST_VERSION;
  std::signal(SIGINT, on_signal);
  std::signal(SIGTERM, on_signal);
  std::signal(SIGWINCH, on_signal);

#ifndef NDEBUG
  log_write("Running a DEBUG build!\n");
#endif

  try {
    Application app;
    app.run();
  }
  catch (const std::exception &e) {
    ::endwin();
    std::printf("%s\n", e.what());
    return 1;
  }

  return 0;
}

