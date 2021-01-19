#ifndef ACTIONS_HPP
#define ACTIONS_HPP

#include "application.hpp"

#include <string>

class Actions {
public:
  enum ActionID : unsigned char {
#define XACTIONS \
  X(NONE,                   "none")                   \
  X(UP,                     "up")                     \
  X(DOWN,                   "down")                   \
  X(PAGE_UP,                "page_up")                \
  X(PAGE_DOWN,              "page_down")              \
  X(TOP,                    "top")                    \
  X(BOTTOM,                 "bottom")                 \
  X(SHOW_COVER,             "show_cover")             \
  X(SEARCH_UP,              "search.up")              \
  X(SEARCH_DOWN,            "search.down")            \
  X(SEARCH_NEXT,            "search.next")            \
  X(SEARCH_PREV,            "search.prev")            \
  X(PLAYER_FORWARD,         "player.forward")         \
  X(PLAYER_BACKWARD,        "player.backward")        \
  X(PLAYER_STOP,            "player.stop")            \
  X(PLAYER_TOGGLE,          "player.toggle")          \
  X(PLAYINGINFO_TOGGLE,     "infoline.toggle")        \
  X(PROGRESSBAR_TOGGLE,     "progressbar.toggle")     \
  X(TABBAR_TOGGLE,          "tabbar.toggle")          \
  X(TABS_NEXT,              "tabs.next")              \
  X(TABS_PREV,              "tabs.prev")              \
  X(SPLASH_SHOW,            "splash.show")            \
  X(PLAYLIST_SHOW,          "playlist.show")          \
  X(BROWSER_SHOW,           "browser.show")           \
  X(HELP_SHOW,              "help.show")              \
  X(INFO_SHOW,              "info.show")              \
  X(PLAYLIST_PLAY,          "playlist.play")          \
  X(PLAYLIST_DOWNLOAD,      "playlist.download")      \
  X(PLAYLIST_NEXT,          "playlist.next")          \
  X(PLAYLIST_PREV,          "playlist.prev")          \
  X(PLAYLIST_CLEAR,         "playlist.clear")         \
  X(PLAYLIST_DELETE,        "playlist.delete")        \
  X(PLAYLIST_GOTO_CURRENT,  "playlist.goto_current")  \
  X(UPDATE,                 "database.update")        \
  X(REDRAW,                 "redraw")                 \
  X(QUIT,                   "quit")
#define X(ENUM, STR) ENUM,
  XACTIONS
#undef X
  ACTIONID_COUNT
  };

  static int call(ActionID);
  static ActionID parse(const std::string&);
  static const char* to_string(ActionID);
};

#endif
