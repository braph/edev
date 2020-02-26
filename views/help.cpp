#include "help.hpp"

#include "../theme.hpp"
#include "../colors.hpp"
#include "../common.hpp"
#include "../bindings.hpp"
#include "../actions.hpp"

#define FG(COLOR) UI::Colors::set(COLOR, -1, 0)
using namespace UI;
using namespace Views;

void Help :: layout(Pos pos, Size size) {
  this->pos  = pos;
  this->size = size;
  wresize(win, 20, size.width);
  pad_minrow = 0;
  pad_mincol = 0;
}

void Help :: draw() {
  wclear(win);
  wmove(win, 0, 0);
  waddstr(win, "Global");
  wmove(win, 1, 0);

  for (int id = 1; id < Actions::ACTIONID_LAST; ++id) {
    int nkeys = 0;

    for (size_t i = 0; i < KEY_MAX; ++i) {
      if (Bindings::global[i] == id) {
        if (nkeys++)
          waddstr(win, ", ");
        waddstr(win, keyname(i));
      }
    }

    if (nkeys) {
      mvwaddstr(win, getcury(win), 30, Actions::to_string(static_cast<Actions::ActionID>(id)));
      wmove(win, getcury(win) + 1, 0);
      //waddch(win, '\n');
    }
  }
}

#ifdef TEST_HELP
#include "../test.hpp"
int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Bindings::init();

  //Theme::current = colors;

  Widget *s = new Views::Help;
  s->layout({10,10}, {30,80});
  s->draw();
  s->noutrefresh();
  doupdate();
  wgetch(s->active_win());

  TEST_END();
}
#endif
