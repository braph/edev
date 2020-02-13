#include "info.hpp"

#include "../theme.hpp"
#include "../colors.hpp"
#include "../common.hpp"

#define FG(COLOR) UI::Colors::set(COLOR, -1, 0)
using namespace UI;
using namespace Views;

void Info :: layout(Pos pos, Size size) {
  this->pos  = pos;
  this->size = size;
  wresize(win, 20, size.width);
  pad_minrow = 0;
  pad_mincol = 0;
}

void Info :: draw() {
  wclear(win);
  wmove(win, 0, 0);
  waddstr(win, "INFO TODO!");
}

#if TEST_INFO
#include "../test.hpp"
int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Bindings::init();

  //Theme::current = colors;

  Widget *s = new Views::Info;
  s->layout({10,10}, {30,80});
  s->draw();
  s->noutrefresh();
  doupdate();
  wgetch(s->active_win());

  TEST_END();
}
#endif
