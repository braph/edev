#include "actions.hpp"

#include "common.hpp"
#include "player.hpp"
#include "database.hpp"
#include "trackloader.hpp"
#include "views/mainwindow.hpp"

static const char *action_strings[Actions::ACTIONID_LAST] = {
#define X(ENUM, STR) STR,
  XActions
#undef X
};

int Actions :: call(ActionID id) {
  int index;

  switch (id) {
  case NONE:   break;
  case QUIT:   return QUIT;
  case REDRAW: return REDRAW;

  // Silence warnings
  case TOP: case BOTTOM: case UP: case DOWN: case PAGE_UP: case PAGE_DOWN:
  case ACTIONID_LAST: break;

  case PLAYER_FORWARD:     p->seek_forward(10);                             break;
  case PLAYER_BACKWARD:    p->seek_backward(10);                            break;
  case PLAYER_STOP:        p->stop();                                       break;
  case PLAYER_TOGGLE:      p->toggle();                                     break;
  case TABBAR_TOGGLE:      v->tabBar.visible      = !v->tabBar.visible;      return REDRAW;
  case PLAYINGINFO_TOGGLE: v->playingInfo.visible = !v->playingInfo.visible; return REDRAW;
  case PROGRESSBAR_TOGGLE: v->progressBar.visible = !v->progressBar.visible; return REDRAW;

  // === Playlist =============================================================
  case PLAYLIST_GOTO_CURRENT: v->playlist.gotoSelected(); break;
  case PLAYLIST_NEXT: index = v->playlist.getActiveIndex() + 1; goto PLAYLIST_PLAY;
  case PLAYLIST_PREV: index = v->playlist.getActiveIndex() - 1; goto PLAYLIST_PLAY;
  case PLAYLIST_PLAY: index = v->playlist.getSelected();
PLAYLIST_PLAY:        v->playlist.setActiveIndex(index);
                      if (! v->playlist.empty() && v->playlist.getActiveIndex() >= 0) {
                        auto track = v->playlist.getActiveItem();
                        p->play(t->getFileForTrack(track, false));
                      }
                      break;

  // === Tabs =================================================================
  case TABS_NEXT:     index = v->windows.currentIndex() + 1;    goto SELECT_TAB;
  case TABS_PREV:     index = v->windows.currentIndex() - 1;    goto SELECT_TAB;
  case SPLASH_SHOW:   index = v->windows.indexOf(&v->splash);   goto SELECT_TAB;
  case PLAYLIST_SHOW: index = v->windows.indexOf(&v->playlist); goto SELECT_TAB;
  case BROWSER_SHOW:  index = v->windows.indexOf(&v->playlist); goto SELECT_TAB; // TODO
  case INFO_SHOW:     index = v->windows.indexOf(&v->info);     goto SELECT_TAB;
  case HELP_SHOW:     index = v->windows.indexOf(&v->help);     goto SELECT_TAB;
SELECT_TAB:           if (index < 0) index = v->windows.count() - 1;
                      else if (index >= v->windows.count()) index = 0;
                      v->windows.setCurrentIndex(index);
                      v->tabBar.setCurrentIndex(index);
                      break;

  // === Error ================================================================
  //default: assert_not_reached();
  }

  return 0;
}

Actions::ActionID Actions :: parse(const std::string &s) {
  for (size_t i = 0; i < Actions::ACTIONID_LAST; ++i)
    if (s == action_strings[i])
      return static_cast<Actions::ActionID>(i);
  return Actions::ACTIONID_LAST;
}

const char* Actions :: to_string(Actions::ActionID id) {
  return action_strings[id];
}
