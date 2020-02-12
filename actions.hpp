#ifndef _ACTIONS_HPP
#define _ACTIONS_HPP

#include "player.hpp"
#include "database.hpp"
#include "views/mainwindow.hpp"

#include <string>

#define XActions \
  X(NONE,               "none")               \
  X(QUIT,               "quit")               \
  X(REDRAW,             "redraw")             \
  X(PLAYER_FORWARD,     "player.forward")     \
  X(PLAYER_BACKWARD,    "player.backward")    \
  X(PLAYER_STOP,        "player.stop")        \
  X(PLAYER_TOGGLE,      "player.toggle")      \
  X(PLAYLIST_NEXT,      "playlist.next")      \
  X(PLAYLIST_PREV,      "playlist.prev")      \
  X(PLAYINGINFO_TOGGLE, "playinginfo.toggle") \
  X(PROGRESSBAR_TOGGLE, "progressbar.toggle") \
  X(TABBAR_TOGGLE,      "tabbar.toggle")      \

class Actions {
public:
#define X(ENUM, STR) ENUM,
  enum ActionID { XActions ACTIONID_LAST };
#undef X
  Actions(Views::MainWindow&, Database&, Mpg123Player&);
  int call(ActionID);
  static ActionID parse(const std::string&);
  static const char* to_string(ActionID);

private:
  Database &db;
  Mpg123Player &p;
  Views::MainWindow &v;
};

#endif
