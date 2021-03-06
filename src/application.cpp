#include "application.hpp"

#include "ektoplayer.hpp"
#include "trackloader.hpp"
#include "database.hpp"
#include "bindings.hpp"
#include "updater.hpp"
#include "config.hpp"
#include "theme.hpp"
#include "mpg123playback.hpp"
#include "views/mainwindow.hpp"

#include <lib/string.hpp>
#include <lib/filesystem.hpp>
#include <lib/cfile.hpp>

#include <libxml/xmlversion.h>

#include <clocale>
#include <csignal>

Database::Database database;
Updater updater(database);
Mpg123Playback player;
TrackLoader trackloader;
Views::MainWindow* mainwindow;

static volatile int caught_signal;
static void on_signal(int sig) { caught_signal = sig; }

class Application {
public:
  Application();
 ~Application();
  void init();
  void run();

private:
  void print_db_stats();
  void delete_stale_download_files();
};

Application :: Application()
{
  init();
}

Application :: ~Application() {
  ::endwin();
  delete_stale_download_files();

  try {
    // Write unoptimized database just in case shrink() fails
    database.save(Config::database_file);
    database.shrink_to_fit();
    database.save(Config::database_file);
  } catch (const std::exception& e) {
    pprintf("Error saving database to file: %s\n", e);
  }

  log_write("Terminated gracefully.\n");
}

void Application :: init() {
  namespace fs = Filesystem;
  const char* e;

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

    Config::init();
    updater.downloads().parallel(10);

    e = "Error while reading configuration file";
    if (fs::exists(Ektoplayer::config_file()))
      Config::read(Ektoplayer::config_file().c_str());

    e = "Could not create config directory";
    fs::create_directories(Ektoplayer::config_dir());

    e = "Could not create cache directory";
    if (! fs::is_directory(Config::cache_dir))
      fs::create_directory(Config::cache_dir);

    e = "Could not create album_dir";
    if (! fs::is_directory(Config::album_dir))
      fs::create_directory(Config::album_dir);

    e = "Could not create archive_dir";
    if (! fs::is_directory(Config::archive_dir))
      fs::create_directory(Config::archive_dir);

    e = "Error opening log file";
    CFile::stderr().reopen(Config::log_file, "a");
    CFile::stderr().setlinebuf();

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

    if (Config::use_colors < 0)
      Config::use_colors = COLORS;

    load_theme_by_colors(Config::use_colors, colors);
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
  ::mainwindow = &mainwindow;

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
      && mainwindow.playlist.list_size() >= 2)
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
  if (! win)
    win = stdscr;

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

void Application :: delete_stale_download_files() {
  Filesystem::error_code e;
  for (auto dir : {&Config::cache_dir, &Config::archive_dir})
    for (const auto& f : Filesystem::directory_iterator(*dir, e))
      if (ends_with(f.path().filename(), EKTOPLAZM_DOWNLOAD_SUFFIX))
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

int main() try {
  LIBXML_TEST_VERSION;
  std::signal(SIGINT, on_signal);
  std::signal(SIGTERM, on_signal);
  std::signal(SIGWINCH, on_signal);

#ifndef NDEBUG
  log_write("Running a DEBUG build!\n");
#endif

  Application().run();
}
catch (const std::exception &e) {
  ::endwin();
  pprintf("%s\n", e);
  return 1;
}
