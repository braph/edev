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

Actions::ActionID Actions :: parse(const std::string &s) {
  for (size_t i = 0; i < Actions::ACTIONID_LAST; ++i)
    if (s == action_strings[i])
      return (Actions::ActionID) i;
  return Actions::ACTIONID_LAST;
}

const char* Actions :: to_string(Actions::ActionID id) {
  return action_strings[id];
}
