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

#include <libxml/xmlversion.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>

#include <locale>
#include <fstream>
#include <csignal>

using namespace Ektoplayer;
namespace fs = boost::filesystem;

static volatile int currentSIGNAL;
static void on_SIGNAL(int sig) { currentSIGNAL = sig; }

class Application {
public:
  Application();
 ~Application();
  void run();
  void init();
private:
  Database database;
  Downloads downloads;
  Updater updater;
  TrackLoader trackloader;
  Mpg123Player player;
  const char* error;

  void printDBStats();
  void cleanup_files();
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
  std::cout // Set terminal title
    << "\033]0;ektoplayer\007" // *xterm
    << "\033kektoplayer\033\\" // screen/tmux
    << "\r";

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
  Theme::loadTheme(Config::use_colors != -1 ? Config::use_colors : COLORS);

  player.audio_system = Config::audio_system;
}

void Application :: run() {
  printDBStats();

  if (database.tracks.size() < 42)
    updater.start(0); // Fetch all pages
  else if (Config::small_update_pages > 0)
    updater.start(-Config::small_update_pages); // Fetch last N pages

  Actions actions;
  Views::MainWindow mainwindow(actions, database, player);
  actions.db = &database;
  actions.p  = &player;
  actions.v  = &mainwindow;
  actions.t  = &trackloader;

  // Connecting widgets events
  mainwindow.progressBar.percentChanged = [&](float percent) {
    player.setPostionByPercent(percent);
    mainwindow.progressBar.setPercent(percent);
  };

  mainwindow.tabBar.indexChanged = [&](int index) {
    mainwindow.tabBar.setCurrentIndex(index);
    mainwindow.windows.setCurrentIndex(index);
  };

  int key;
  int timeOut;
  int downloading;
  WINDOW *win;
  MEVENT mouse;
  Database::Tracks::Track nextTrack(database, 0); // "NULL" rows
  Database::Tracks::Track currentPrefetching(database, 0);

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

  // First thing we do is asking the player do update its properties.
  // Since that update is done in a seperate thread using async I/O, we want
  // to do as much other work as possible before accessing the properties.
  player.work();

  for (downloading = 0; downloads.work() && ++downloading < 50;);

  // Song prefetching
  if (Config::prefetch
      && player.getState() == Mpg123Player::PLAYING
      && player.length() >= 30
      && player.percent() >= 0.5
      && mainwindow.playlist.containerSize() >= 2)
  {
    auto list = mainwindow.playlist.getList();
    nextTrack = (*list)[size_t(mainwindow.playlist.getActiveIndex() + 1) % list->size()];
    if (nextTrack != currentPrefetching) {
      trackloader.getFileForTrack(nextTrack);
      currentPrefetching = nextTrack;
    }
  }

  if (player.isTrackCompleted())
    actions.call(Actions::PLAYLIST_NEXT);

  mainwindow.progressBar.setPercent(player.percent());
  mainwindow.playingInfo.setPositionAndLength(player.position(), player.length());
  mainwindow.playingInfo.setState(player.getState());
  if (!mainwindow.playlist.empty() && mainwindow.playlist.getActiveIndex() >= 0) {
    mainwindow.playingInfo.setTrack(mainwindow.playlist.getActiveItem());
    mainwindow.info.setCurrentTrack(mainwindow.playlist.getActiveItem());
  }
  mainwindow.noutrefresh();
  doupdate();

  if (downloading)
    timeOut = 100; // Set a short timeout if the mainloop handles downloads
  else if (player.getState() == Mpg123Player::STOPPED || player.getState() == Mpg123Player::PAUSED)
    timeOut = -1; // Stop the mainloop until user hits a key
  else
    timeOut = 1000; // In playing state we need 

  win = mainwindow.getWINDOW();
  wtimeout(win, timeOut);
  switch ((key = wgetch(win))) {
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
  boost::system::error_code e;
  for (auto& f : boost::filesystem::directory_iterator(Config::temp_dir, e))
    if (boost::algorithm::starts_with(f.path().filename().string(), "~ekto-"))
      boost::filesystem::remove(f.path(), e);
}

void Application :: printDBStats() {
  std::cerr << "Database statistics:\n";
  for (auto table : database.tables) {
    std::cerr << table->name << "(" << table->size() << "): ";
    for (auto column : table->columns)
      std::cerr << column->bits() << "|";
    std::cerr << "\n";
  }
}

int main() {
  LIBXML_TEST_VERSION /* Check for ABI mismatch */
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

