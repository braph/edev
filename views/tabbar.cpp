#include "tabbar.hpp"

#include "../theme.hpp"

using namespace Views;

TabBar :: TabBar()
: UI::Window(), _current(0)
{
}

void TabBar :: addTab(const std::string &label) {
  _tabs.push_back(label);
  draw();
}

void TabBar :: setCurrentIndex(int index) {
  _current = index % _tabs.size();
  draw();
}

int TabBar :: currentIndex() {
  return _current;
}

void TabBar :: layout(UI::Pos pos, UI::Size size) {
  size.height = 1;
  if (size != this->size) {
    this->size = size;
    wresize(win, size.height, size.width);
  }
  if (pos != this->pos) {
    this->pos = pos;
    mvwin(win, pos.y, pos.x);
  }
}

void TabBar :: draw() {
  werase(win);
  wmove(win, 0, 0);

  unsigned int i = 0;
  for (const auto &label : _tabs) {
    if (i++ == _current)
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
    b.addTab(s);

  for (;;)
    for (int i = 0; i < 3; ++i) {
      b.setCurrentIndex(i);
      b.noutrefresh();
      doupdate();
      usleep(500 * 1000);
    }

  TEST_END();
}
#endif
