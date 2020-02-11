#include "tabbar.hpp"

#include "../theme.hpp"

using namespace Views;

TabBar :: TabBar()
: UI::Window(), current(0)
{
}

void TabBar :: add(const std::string &label) {
  tabs.push_back(label);
  draw();
}

void TabBar :: select(unsigned int index) {
  index = index % tabs.size();
  current = index;
  draw();
}

void TabBar :: layout(UI::Pos pos, UI::Size size) {
  size.height = 1;
  this->pos = pos;
  this->size = size;
  wresize(win, size.height, size.width);
  mvwin(win, pos.y, pos.x);
}

void TabBar :: draw() {
  werase(win);
  wmove(win, 0, 0);

  unsigned int i = 0;
  for (const auto &label : tabs) {
    if (i++ == current)
      wattrset(win, Theme::get(Theme::TABBAR_SELECTED));
    else
      wattrset(win, Theme::get(Theme::TABBAR_UNSELECTED));

    waddch(win, ' ');
    waddstr(win, label.c_str());
  }
}

#if 0
void clicked(...) {
  int x, y;

  // TODO: check for button?

  unsigned int i = 0;
  unsigned int p = 0;
  for (const auto &label : tabs) {
    if (x >= p && x <= p + label.size()) {
      if (changed)
        changed(...);
      break;
    }
    i++;
    p += label.size() + 1;
  }
}
#endif

#if TEST_TABBAR
#include "../test.hpp"
int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Theme::loadTheme(256);

  TabBar b;
  b.layout({0,0}, {LINES,COLS});
  for (auto s : {"Tab1", "Tab2", "Tab3"})
    b.add(s);

  for (;;)
    for (int i = 0; i < 3; ++i) {
      b.select(i);
      b.noutrefresh();
      doupdate();
      sleep(1);
    }

  TEST_END();
}
#endif
