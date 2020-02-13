#include "actions.hpp"
#include "common.hpp"

static const char *action_strings[Actions::ACTIONID_LAST] = {
#define X(ENUM, STR) STR,
  XActions
#undef X
};

Actions :: Actions(Views::MainWindow& v, Database& db, Mpg123Player& p)
: db(db), p(p), v(v)
{
}

int Actions :: call(ActionID id) {
  int index;

  switch (id) {
  case QUIT:   return QUIT;
  case REDRAW: return REDRAW;

  case PLAYER_FORWARD:     p.seek_forward(10);                             break;
  case PLAYER_BACKWARD:    p.seek_backward(10);                            break;
  case PLAYER_STOP:        p.stop();                                       break;
  case PLAYER_TOGGLE:      p.toggle();                                     break;
  case TABBAR_TOGGLE:      v.tabBar.visible      = !v.tabBar.visible;      return REDRAW;
  case PLAYINGINFO_TOGGLE: v.playingInfo.visible = !v.playingInfo.visible; return REDRAW;
  case PROGRESSBAR_TOGGLE: v.progressBar.visible = !v.progressBar.visible; return REDRAW;

  // === TABs stuff ===========================================================
  case TABS_NEXT:   index = v.windows.currentIndex() + 1; goto SELECT_TAB;
  case TABS_PREV:   index = v.windows.currentIndex() - 1; goto SELECT_TAB;
  case SPLASH_SHOW: index = v.windows.indexOf(&v.splash); goto SELECT_TAB;
  case INFO_SHOW:   index = v.windows.indexOf(&v.info);   goto SELECT_TAB;
  case HELP_SHOW:   index = v.windows.indexOf(&v.help);   goto SELECT_TAB;
SELECT_TAB:         index %= v.windows.count();
                    v.windows.setCurrentIndex(index);
                    v.tabBar.setCurrentIndex(index);
                    break;
  // ==========================================================================

  default: assert_not_reached();
  }

  return 0;
}

Actions::ActionID Actions :: parse(const std::string &s) {
  for (size_t i = 0; i < Actions::ACTIONID_LAST; ++i)
    if (s == action_strings[i])
      return (Actions::ActionID) i;
  return Actions::ACTIONID_LAST;
}

const char* Actions :: to_string(Actions::ActionID id) {
  return action_strings[id];
}
