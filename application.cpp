#include "ektoplayer.hpp"
#include "database.hpp"
#include "updater.hpp"
#include "config.hpp"
#include "colors.hpp"
#include "theme.hpp"
#include "xml.hpp"

#include <ncurses.h> // TODO: Remove me
#include <unistd.h>  // TODO: Remove me
#include <signal.h>
#include <glob.h>

#include <fstream>
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;
using namespace Ektoplayer;

static void init(Database&);
static void program(Database&);
static void cleanup();
static bool hv_SIGWINCH = false;
static void on_SIGWINCH(int) { hv_SIGWINCH = true; }
static const char* emsg = "Error"; // Better than extending exception messages!

int main(int argc, char **argv) {
  std::string err;
  LIBXML_TEST_VERSION /* Check for ABI mismatch */

  Database db;
  try {
    init(db);
    program(db);
  }
  catch (const std::exception &e) { err = e.what();                 }
  catch (const char *e)           { err = e;                        }
  catch (...)                     { err = "Unknown exception type"; }
  endwin();

  if (! err.empty())
    std::cout << emsg << ": " << err << std::endl;

  try { db.save(Config::database_file); }
  catch (const std::exception &e) {
    std::cout << "Error saving database file: " << e.what() << std::endl;
  }

  return ! err.empty();
}

static void init(Database &db) {
  // Set terminal title
  std::cout << "\033]0;ektoplayer\007" << std::endl;

  // Initialize curses
  initscr();
  cbreak();
  noecho();
  start_color();
  use_default_colors();
  timeout(100);
  mousemask(ALL_MOUSE_EVENTS/*|REPORT_MOUSE_POSITION*/, NULL);

  // Initialize singleton stuff
  UI::Color::init();
  UI::Colors::init();
  UI::Attribute::init();
  Theme::init();
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

  // Reset error message
  emsg = "Error";

  Config::database_file = "/tmp/ekto-test.db"; // XXX TODO!

  if (fs::exists(Config::database_file))
    try { db.load(Config::database_file); }
    catch (const std::exception &e) {
      addstr("Could not load the database file. Press ANY key");
      getch();
    }

  // All colors are beautiful
  if (Config::use_colors == -1 /* "auto" */)
    Theme::loadTheme(COLORS);
  else
    Theme::loadTheme(Config::use_colors);

  atexit(cleanup);
  signal(SIGWINCH, on_SIGWINCH);
}

#include "ui/container.cpp"
#include "views/splash.cpp"
static void program(Database &db) {
  std::cerr << "Database track count on start: " << db.tracks.size() << std::endl;

  Updater updater(db);
  if (db.tracks.size() < 42)
    updater.start(0); // Fetch all pages
  else if (Config::small_update_pages > 0)
    updater.start(-Config::small_update_pages); // Fetch last N pages

  UI::VerticalContainer v_VerticalContainer;
  Views::Splash         v_Splash;
  v_VerticalContainer.add(&v_Splash);

  v_VerticalContainer.layout({0,0}, {LINES,COLS});
  v_VerticalContainer.draw();
  v_VerticalContainer.refresh();

  int c;
MAINLOOP:
  updater.write_to_database();

  c = wgetch(v_VerticalContainer.active_win());
  if (c == 'q')
    goto QUIT;

goto MAINLOOP;

QUIT: (void)0;
}

static void cleanup() {
  // TODO: What about our threads?
  glob_t globbuf;
  char pattern[8192];
  sprintf(pattern, "%s" PATH_SEP "~ekto-*", Config::temp_dir.c_str());
  glob(pattern, 0, NULL, &globbuf);
  while (globbuf.gl_pathc--)
    unlink(globbuf.gl_pathv[globbuf.gl_pathc]);
  globfree(&globbuf);
}

