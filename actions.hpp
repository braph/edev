#ifndef _ACTIONS_HPP
#define _ACTIONS_HPP

class Database;
class TrackLoader;
class Mpg123Player;
namespace Views { class MainWindow; }

#include <string>

#define XActions \
  X(NONE,               "none")               \
  X(QUIT,               "quit")               \
  X(REDRAW,             "redraw")             \
  X(PLAYER_FORWARD,     "player.forward")     \
  X(PLAYER_BACKWARD,    "player.backward")    \
  X(PLAYER_STOP,        "player.stop")        \
  X(PLAYER_TOGGLE,      "player.toggle")      \
  X(PLAYINGINFO_TOGGLE, "playinginfo.toggle") \
  X(PROGRESSBAR_TOGGLE, "progressbar.toggle") \
  X(TABBAR_TOGGLE,      "tabbar.toggle")      \
  X(TABS_NEXT,          "tabs.next")          \
  X(TABS_PREV,          "tabs.prev")          \
  X(SPLASH_SHOW,        "splash.show")        \
  X(PLAYLIST_SHOW,      "playlist.show")      \
  X(BROWSER_SHOW,       "browser.show")       \
  X(HELP_SHOW,          "help.show")          \
  X(INFO_SHOW,          "info.show")          \
  X(PLAYLIST_PLAY,      "playlist.play")      \
  X(PLAYLIST_NEXT,      "playlist.next")      \
  X(PLAYLIST_PREV,      "playlist.prev")      \
  X(PLAYLIST_TOP,       "playlist.top")       \
  X(PLAYLIST_BOTTOM,    "playlist.bottom")    \
  X(PLAYLIST_UP,        "playlist.up")        \
  X(PLAYLIST_DOWN,      "playlist.down")      \
  X(PLAYLIST_PAGE_UP,   "playlist.page_up")   \
  X(PLAYLIST_PAGE_DOWN, "playlist.page_down") \
  X(PLAYLIST_GOTO_CURRENT, "playlist.goto_current") \

class Actions {
public:
#define X(ENUM, STR) ENUM,
  enum ActionID : unsigned char { XActions ACTIONID_LAST };
#undef X
  Actions(Views::MainWindow&, Database&, Mpg123Player&, TrackLoader&);
  int call(ActionID);
  static ActionID parse(const std::string&);
  static const char* to_string(ActionID);

private:
  Database &db;
  Mpg123Player &p;
  Views::MainWindow &v;
  TrackLoader &t;
};

#endif
