#include "ektoplayer.hpp"
#include "database.hpp"
#include "updater.hpp"
#include "config.hpp"
#include "colors.hpp"
#include "theme.hpp"
#include "xml.hpp"

#include "ui/container.hpp"
#include "views/splash.hpp"
#include "views/progressbar.hpp"
#include "views/playinginfo.hpp"

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
static bool hv_SIGWINCH = false;
static void on_SIGWINCH(int) { hv_SIGWINCH = true; }
static const char* emsg;

int main(int argc, char **argv) {
  std::string err;
  LIBXML_TEST_VERSION /* Check for ABI mismatch */

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

  try { database.save(Config::database_file); }
  catch (const std::exception &e) {
    std::cout << "Error saving database file: " << e.what() << std::endl;
  }

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
  timeout(100);
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

  Views::Splash         v_Splash;
  Views::PlayingInfo    v_PlayingInfo(database);
  Views::ProgressBar    v_ProgressBar;
  UI::VerticalContainer v_VerticalContainer;
  v_VerticalContainer.add(&v_PlayingInfo);
  v_VerticalContainer.add(&v_ProgressBar);
  v_VerticalContainer.add(&v_Splash);

  int c;
MAINLOOP:
  updater.write_to_database();

  v_VerticalContainer.layout({0,0}, {LINES,COLS});
  v_VerticalContainer.draw();
  v_VerticalContainer.refresh();

  c = wgetch(v_VerticalContainer.active_win());
  if (c == 'q')
    return;

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

