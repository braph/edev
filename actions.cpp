#include "actions.hpp"
#include "common.hpp"

Actions :: Actions(Views::MainWindow& v, Database& db, Mpg123Player& p)
: v(v), db(db), p(p)
{
}

int Actions :: call(ActionID id) {
  switch (id) {
    case QUIT:   return QUIT;
    case REDRAW: return REDRAW;

    case PLAYER_FORWARD:     p.seek_forward(10);                             break;
    case PLAYER_BACKWARD:    p.seek_backward(10);                            break;
    case TABBAR_TOGGLE:      v.tabBar.visible      = !v.tabBar.visible;      return REDRAW;
    case PLAYINGINFO_TOGGLE: v.playingInfo.visible = !v.playingInfo.visible; return REDRAW;
    case PROGRESSBAR_TOGGLE: v.progressBar.visible = !v.progressBar.visible; return REDRAW;

    default: assert_not_reached();
  }

  return 0;
}
