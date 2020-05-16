#include "application.hpp"

#include "ektoplayer.hpp"
#include "trackloader.hpp"
#include "database.hpp"
#include "bindings.hpp"
#include "updater.hpp"
#include "player.hpp"
#include "config.hpp"
#include "theme.hpp"
#include "log.hpp"
#include "views/mainwindow.hpp"

#include "lib/cstring.hpp"

#include <libxml/xmlversion.h>

#include <clocale>
#include <csignal>
#include <type_traits>

namespace fs = Filesystem;

static volatile int caught_signal;
static void on_signal(int sig) { caught_signal = sig; }

class Application {
public:
  Application();
 ~Application();
  void init();
  void run();
private:
  Database::Database database;
  Updater updater;
  TrackLoader trackloader;
  Mpg123Player player;
  Context ctxt;
  const char* error;

  void print_db_stats();
  void cleanup_files();
};

Application :: Application()
: database()
, updater(database)
, trackloader()
, player()
{
  try {
    init();
  }
  catch (const std::exception &e) {
    throw std::runtime_error(std::string(error) + ": " + e.what());
  }

  ctxt.player = &player;
  ctxt.database = &database;
  ctxt.trackloader = &trackloader;
}

Application :: ~Application() {
  endwin();
  cleanup_files();

  const char* err;
  // Write unoptimized database just in case shrink() fails
  if (! (err = database.save(Config::database_file))) {
    database.shrink_to_fit();
    err = database.save(Config::database_file);
  }
  if (err)
    printf("Error saving database to file: %s\n", err);

  log_write("Terminated gracefully.\n");
}

void Application :: init() {
  // Set terminal title
  printf("\033]0;ektoplayer\007" // *xterm
         "\033kektoplayer\033\\" // screen/tmux
         "\r\n");
 
  // Use the locale from the environment
  std::setlocale(LC_ALL, "");

  // Initialize curses
  initscr();
  cbreak();
  noecho();
  start_color();
  use_default_colors();
  curs_set(0);
  mousemask(ALL_MOUSE_EVENTS, NULL);
  wresize(stdscr, 1, 1); // Save some bytes...

  error = REPORT_BUG;
  Bindings::init();
  Config::init();

  error = "Error while reading configuration file";
  if (fs::exists(Ektoplayer::config_file()))
    Config::read(Ektoplayer::config_file().string());

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
  if (! std::freopen(Config::log_file.c_str(), "a", stderr))
    throw std::runtime_error(std::strerror(errno));
  setvbuf(stderr, NULL, _IOLBF, 0);

  error = "Error opening database file. Try again, then delete it. Sorry!";
  if (fs::exists(Config::database_file)) {
    const char* err = database.load(Config::database_file);
    if (err)
      throw std::runtime_error(err);
  }
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
  Theme::loadThemeByColors(Config::use_colors != -1 ? Config::use_colors : COLORS);

  updater.downloads().setParallel(10);
}

void Application :: run() {
  print_db_stats();

  if (database.tracks.size() < 42)
    updater.start(); // Fetch all pages
  //else if (Config::small_update_pages > 0)
  //  updater.start(Config::small_update_pages); // Fetch last N pages

  Views::MainWindow mainwindow(ctxt);
  ctxt.mainwindow = &mainwindow;

  // Connecting widgets events
  mainwindow.progressBar.percentChanged = [&](float percent) {
    player.percent(percent);
    mainwindow.progressBar.setPercent(percent);
  };

  mainwindow.tabBar.indexChanged = [&](int index) {
    mainwindow.tabBar.setCurrentIndex(index);
    mainwindow.windows.setCurrentIndex(index);
  };

  int key;
  WINDOW *win;
  MEVENT mouse;
  Database::Tracks::Track nextTrack;
  Database::Tracks::Track currentPrefetching;

  mainwindow.playlist.playlist = database.getTracks();

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
  if (Config::prefetch
      && player.isPlaying()
      && player.length() >= 30
      && player.percent() >= 0.5
      && mainwindow.playlist.containerSize() >= 2)
  {
    auto list = mainwindow.playlist.list();
    nextTrack = (*list)[size_t(mainwindow.playlist.activeIndex() + 1) % list->size()];
    if (nextTrack != currentPrefetching) {
      trackloader.getFileForTrack(nextTrack);
      currentPrefetching = nextTrack;
    }
  }

  if (player.isTrackCompleted())
    Actions::call(ctxt, Actions::PLAYLIST_NEXT);

  mainwindow.progressBar.setPercent(player.percent());
  mainwindow.infoLine.setPositionAndLength(player.position(), player.length());
  mainwindow.infoLine.setState(player.state());
  if (!mainwindow.playlist.empty() && mainwindow.playlist.activeIndex() >= 0) {
    mainwindow.infoLine.setTrack(mainwindow.playlist.getActiveItem());
    mainwindow.info.setCurrentTrack(mainwindow.playlist.getActiveItem());
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

  if (trackloader.downloads().runningDownloads() || trackloader.downloads().queuedDownloads()
      ||  updater.downloads().runningDownloads() || updater.downloads().queuedDownloads())
    wtimeout(win, 100); // Short timeout, want to continue downloading soon
  else if (player.isStopped() || player.isPaused())
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
        mainwindow.handleMouse(mouse);
      break;
    default:
      mainwindow.handleKey(key);
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
    endwin();
    std::printf("%s\n", e.what());
    return 1;
  }

  return 0;
}

