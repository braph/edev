#define ACTIONS_EXPORT_XACTIONS
#include "actions.hpp"

#include "player.hpp"
#include "database.hpp"
#include "trackloader.hpp"
#include "views/mainwindow.hpp"

#include <csignal>
#include <cassert>

int Actions :: call(Context& ctxt, ActionID id) {
  int index;

  switch (id) {
  case NONE:
    break;
  case QUIT:
    std::raise(SIGTERM);
    break;
  case REDRAW:
    std::raise(SIGWINCH);
    break;

  // Player
  case PLAYER_FORWARD:
    ctxt.player->seekForward(10);
    break;
  case PLAYER_BACKWARD:
    ctxt.player->seekBackward(10);
    break;
  case PLAYER_STOP:
    ctxt.player->stop();
    break;
  case PLAYER_TOGGLE:
    ctxt.player->toggle();
    break;

  // Widgets visibility
  case TABBAR_TOGGLE:
    ctxt.mainwindow->tabBar.visible = !ctxt.mainwindow->tabBar.visible;
    break;
  case PLAYINGINFO_TOGGLE:
    ctxt.mainwindow->infoLine.visible = !ctxt.mainwindow->infoLine.visible;
    break;
  case PROGRESSBAR_TOGGLE:
    ctxt.mainwindow->progressBar.visible = !ctxt.mainwindow->progressBar.visible;
    break;

  // Playlist
  case PLAYLIST_GOTO_CURRENT:
    ctxt.mainwindow->playlist.gotoSelected();
    break;
  case PLAYLIST_NEXT:
    index = ctxt.mainwindow->playlist.activeIndex() + 1;
    goto PLAYLIST_PLAY;
  case PLAYLIST_PREV:
    index = ctxt.mainwindow->playlist.activeIndex() - 1;
    goto PLAYLIST_PLAY;
  case PLAYLIST_PLAY:
    index = ctxt.mainwindow->playlist.cursorIndex();
    goto PLAYLIST_PLAY;

PLAYLIST_PLAY:
    ctxt.mainwindow->playlist.activeIndex(index);
    if (! ctxt.mainwindow->playlist.empty() && ctxt.mainwindow->playlist.activeIndex() >= 0) {
      auto track = ctxt.mainwindow->playlist.getActiveItem();
      ctxt.player->play(ctxt.trackloader->getFileForTrack(track, false));
    }
    break;

  // Windows
  case TABS_NEXT:
    index = ctxt.mainwindow->windows.currentIndex() + 1;
    goto SELECT_TAB;
  case TABS_PREV:
    index = ctxt.mainwindow->windows.currentIndex() - 1;
    goto SELECT_TAB;
  case SPLASH_SHOW:
    index = ctxt.mainwindow->windows.indexOf(&ctxt.mainwindow->splash);
    goto SELECT_TAB;
  case PLAYLIST_SHOW:
    index = ctxt.mainwindow->windows.indexOf(&ctxt.mainwindow->playlist);
    goto SELECT_TAB;
  case BROWSER_SHOW:
    index = ctxt.mainwindow->windows.indexOf(&ctxt.mainwindow->playlist); // TODO [later]
    goto SELECT_TAB;
  case INFO_SHOW:
    index = ctxt.mainwindow->windows.indexOf(&ctxt.mainwindow->info);
    goto SELECT_TAB;
  case HELP_SHOW:
    index = ctxt.mainwindow->windows.indexOf(&ctxt.mainwindow->help);
    goto SELECT_TAB;

SELECT_TAB:
    if (index < 0)
      index = ctxt.mainwindow->windows.count() - 1;
    else if (index >= ctxt.mainwindow->windows.count())
      index = 0;
    ctxt.mainwindow->windows.setCurrentIndex(index);
    ctxt.mainwindow->tabBar.setCurrentIndex(index);
    break;

  // Silence warnings
  case UP:
  case DOWN:
  case PAGE_UP:
  case PAGE_DOWN:
  case TOP:
  case BOTTOM:
  case SEARCH:
  case SEARCH_PREV:
  case SEARCH_NEXT:
  case ACTIONID_ENUM_LAST: break;
  default: assert(0);
  }

  return 0;
}

static const char *action_strings[Actions::ACTIONID_ENUM_LAST] = {
#define X(ENUM, STR) STR,
  XACTIONS
};

Actions::ActionID Actions :: parse(const std::string& s) {
  for (size_t i = 0; i < Actions::ACTIONID_ENUM_LAST; ++i)
    if (s == action_strings[i])
      return static_cast<Actions::ActionID>(i);
  return Actions::ACTIONID_ENUM_LAST;
}

const char* Actions :: to_string(Actions::ActionID id) {
  return action_strings[id];
}

