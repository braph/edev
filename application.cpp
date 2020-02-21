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

#include <locale>
#include <fstream>

using namespace Ektoplayer;
namespace fs = boost::filesystem;

static bool have_SIGWINCH;
static void on_SIGWINCH(int) { have_SIGWINCH = true; }
static void printDBStats(Database&);

class Application {
public:
  Application();
 ~Application();
  void run();
  void init();
  void cleanup_files();
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
  cleanup_files();

  try {
    database.shrink_to_fit();
    database.save(Config::database_file);
  }
  catch (const std::exception &e) {
    endwin();
    std::cout << "Error saving database to file: " << e.what() << std::endl;
  }
}

void Application :: init() {
  // Set terminal title
  std::cout << "\033]0;ektoplayer\007" << std::endl;

  // Use the locale from the environment?!
  std::locale::global(std::locale(""));

  // Initialize curses
  initscr();
  //savetty();
  cbreak();
  noecho();
  start_color();
  use_default_colors();
  curs_set(0);
  mousemask(ALL_MOUSE_EVENTS/*|REPORT_MOUSE_POSITION*/, NULL);

  error = REPORT_BUG;
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
}

void Application :: run() {
  printDBStats(database);
  std::cerr << "Database track count: " << database.tracks.size() << std::endl;

  if (database.tracks.size() < 42)
    updater.start(0); // Fetch all pages
  else if (Config::small_update_pages > 0)
    updater.start(-Config::small_update_pages); // Fetch last N pages

  //player.audio_system = Config::audio_system;
  Views::MainWindow mainwindow(database);
  Actions actions(mainwindow, database, player, trackloader);

  // Connecting widgets events
  mainwindow.progressBar.percentChanged = [&](float f) {
    player.setPostionByPercent(f);
    mainwindow.progressBar.setPercent(f);
  };
  mainwindow.tabBar.indexChanged = [&](int idx) {
    mainwindow.tabBar.setCurrentIndex(idx);
    mainwindow.windows.setCurrentIndex(idx);
  };

  int c;
  int timeOut = 0;
  int downloading;
  WINDOW *win;
  MEVENT mouse;
  Database::Tracks::Track nextTrack(database, 0); // "NULL" rows
  Database::Tracks::Track currentPrefetching(database, 0);
  auto tracks = database.getTracks();
#define TIMEOUT_PLAYING  1000 // Display refresh rate if playing
#define TIMEOUT_DOWNLOAD 100  // Timeout working downloads

WINDOW_RESIZE:
  have_SIGWINCH = false;
  mainwindow.layout({0,0}, {LINES,COLS});
  mainwindow.draw();

MAINLOOP:
  if (have_SIGWINCH)
    goto WINDOW_RESIZE;

  mainwindow.progressBar.setPercent(player.percent());
  mainwindow.playingInfo.setPositionAndLength(player.position(), player.length());
  mainwindow.playingInfo.setState(player.getState());
  mainwindow.playingInfo.setTrack(mainwindow.playlist.getActiveItem()); //XXX bounds
  mainwindow.noutrefresh();
  doupdate();

  // Player stuff
  player.work();
  if (player.isTrackCompleted())
    actions.call(Actions::PLAYLIST_NEXT);

  // Do at max 10 download iterations
  for (downloading = 0; downloading < 100 && downloads.work(); ++downloading);

  // Song prefetching
  if (Config::prefetch &&
      player.getState() == Mpg123Player::PLAYING &&
      player.length() >= 30 &&
      player.percent() >= 0.5 &&
      mainwindow.playlist.getList() &&
      mainwindow.playlist.getList()->size() >= 2
      ) {
    auto list = mainwindow.playlist.getList();
    nextTrack = (*list)[(mainwindow.playlist.getActiveIndex() + 1) % list->size()];
    if (nextTrack != currentPrefetching) {
      trackloader.getFileForTrack(nextTrack);
      currentPrefetching = nextTrack;
    }
  }

  if (downloading)
    timeOut = TIMEOUT_DOWNLOAD;
  else if (player.getState() == Mpg123Player::STOPPED || player.getState() == Mpg123Player::PAUSED)
    timeOut += TIMEOUT_PLAYING;
  else
    timeOut = TIMEOUT_PLAYING;

  win = mainwindow.active_win();
  wtimeout(win, timeOut);
  if ((c = wgetch(win)) != ERR) {
    if (c == KEY_MOUSE) {
      if (OK == getmouse(&mouse))
        mainwindow.handleMouse(mouse);
      goto MAINLOOP;
    }

    if (Bindings::global[c])
      c = Bindings::global[c];
    else if (Bindings::playlist[c])
      c = Bindings::playlist[c];
    else if (c == 'x') {
      database.shrink_to_fit();
      tracks = database.getTracks();
      mainwindow.playlist.attachList(&tracks);
      goto MAINLOOP;
    }
    else
      c = Actions::NONE;

    c = actions.call(static_cast<Actions::ActionID>(c));
    if (c == Actions::QUIT)
      return;
    else if (c == Actions::REDRAW)
      goto WINDOW_RESIZE;
  }

goto MAINLOOP;
}

void Application :: cleanup_files() {
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

  //resetty();
  reset_shell_mode();
  delwin(stdscr);
  return 0;
}

static void printDBStats(Database& db) {
  std::cerr << "Database statistics:";
  const char* names[] = {"styles", "albums", "tracks"};
  Database::Table* tables[] = {&db.styles, &db.albums, &db.tracks};
  for (size_t i = 0; i < ARRAY_SIZE(tables); ++i) {
    std::cerr << "\n" << names[i] << ":";
    for (auto col : tables[i]->columns)
      std::cerr << col->bits() << "|";
  }
}
