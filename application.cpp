#include "ektoplayer.hpp"
#include "trackloader.hpp"
#include "lib/downloads.hpp"
#include "database.hpp"
#include "bindings.hpp"
#include "updater.hpp"
#include "player.hpp"
#include "config.hpp"
#include "context.hpp"
#include "theme.hpp"
#include "views/mainwindow.hpp"

#include <libxml/xmlversion.h>

#include <boost/algorithm/string/predicate.hpp>

#include <locale>
#include <fstream>
#include <csignal>
#include <iostream>

namespace fs = Filesystem;

static volatile int currentSIGNAL;
static void on_SIGNAL(int sig) { currentSIGNAL = sig; }

class Application {
public:
  Application();
 ~Application();
  void run();
  void init();
private:
  Database::Database database;
  Downloads downloads;
  Updater updater;
  TrackLoader trackloader;
  Mpg123Player player;
  Context ctxt;
  const char* error;

  void printDBStats();
  void cleanup_files();
};

Application :: Application()
: database()
, downloads(2)
, updater(database, downloads)
, trackloader(downloads)
, player()
{
  try { init(); }
  catch (const std::exception &e) {
    throw std::runtime_error(std::string(error) + ": " + e.what());
  }

  ctxt.database = &database;
  ctxt.trackloader = &trackloader;
  ctxt.downloads = &downloads;
  ctxt.player = &player;
}

Application :: ~Application() {
  endwin();
  cleanup_files();

  try {
    database.shrink_to_fit();
    database.save(Config::database_file);
  }
  catch (const std::exception &e) {
    std::cout << "Error saving database to file: " << e.what() << std::endl;
  }

  std::cerr << "Terminated gracefully.\n";
}

void Application :: init() {
  std::ios::sync_with_stdio(false);
  std::cout << // Set terminal title
    "\033]0;ektoplayer\007" // *xterm
    "\033kektoplayer\033\\" // screen/tmux
    "\r";

  // Use the locale from the environment?!
  std::locale::global(std::locale(""));

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
  std::ofstream *logfile = new std::ofstream(); /* "wanted" leak */
  logfile->exceptions(std::ofstream::failbit|std::ofstream::badbit);
  logfile->open(Config::log_file, std::ofstream::out|std::ofstream::app);
  std::cerr.rdbuf(logfile->rdbuf());

  error = "Error opening database file. Try again, then delete it. Sorry!";
  if (fs::exists(Config::database_file))
    database.load(Config::database_file);
  else {
    // The database will *at least* hold this amount of data
    database.styles.reserve(EKTOPLAZM_STYLE_COUNT);
    database.albums.reserve(EKTOPLAZM_ALBUM_COUNT);
    database.tracks.reserve(EKTOPLAZM_TRACK_COUNT);
    database.pool_meta.reserve(EKTOPLAZM_META_SIZE);
    database.pool_desc.reserve(EKTOPLAZM_DESC_SIZE);
    database.pool_cover_url.reserve(EKTOPLAZM_COVER_URL_SIZE);
    database.pool_album_url.reserve(EKTOPLAZM_ALBUM_URL_SIZE);
    database.pool_track_url.reserve(EKTOPLAZM_TRACK_URL_SIZE);
    database.pool_style_url.reserve(EKTOPLAZM_STYLE_URL_SIZE);
    database.pool_archive_url.reserve(EKTOPLAZM_ARCHIVE_URL_SIZE);
  }

  // All colors are beautiful
  Theme::loadThemeByColors(Config::use_colors != -1 ? Config::use_colors : COLORS);
}

void Application :: run() {
  printDBStats();

  if (database.tracks.size() < 42)
    updater.start(0); // Fetch all pages
  //else if (Config::small_update_pages > 0)
  //  updater.start(-Config::small_update_pages); // Fetch last N pages

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
  currentSIGNAL = 0;
  mainwindow.layout({0,0}, {LINES,COLS});
  mainwindow.draw();

MAINLOOP:
  switch (currentSIGNAL) {
    case SIGINT:
    case SIGTERM: return;
    case SIGWINCH: goto WINDOW_RESIZE;
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
  wtimeout(win, 1);
  while (downloads.work())
    if (((key = wgetch(win)) != ERR))
      goto HANDLE_KEY;

  if (downloads.runningHandles())
    wtimeout(win, 100); // Short timeout, want to continue downloading soon
  else if (player.isStopped() || player.isPaused()) {
    wtimeout(win, -1); // We have *nothing* to do, wait until user hits a key
    std::cerr << "tm is -1\n";
  }
  else
    wtimeout(win, 900); // In playing state we need some UI refreshes

  key = wgetch(win);
HANDLE_KEY:
  switch (key) {
    case ERR: break;
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
    if (boost::algorithm::starts_with(f.path().filename().string(), EKTOPLAZM_TEMP_FILE_PREFIX))
      Filesystem::remove(f.path(), e);
}

void Application :: printDBStats() {
  std::cerr << "Database statistics:\n";
  for (const auto& table : database.tables) {
    std::cerr << table->name << '(' << table->size() << "): ";
    for (const auto& column : table->columns)
      std::cerr << column->bits() << '|';
    std::cerr << '\n';
  }
}

int main() {
  LIBXML_TEST_VERSION;
  std::signal(SIGWINCH, on_SIGNAL);
  std::signal(SIGINT,   on_SIGNAL);
  std::signal(SIGTERM,  on_SIGNAL);

#ifndef NDEBUG
  std::cerr << "Running a DEBUG build!\n";
#endif

  try {
    Application app;
    app.run();
  }
  catch (const std::exception &e) {
    endwin();
    std::cout << e.what() << std::endl;
    return 1;
  }

  return 0;
}

