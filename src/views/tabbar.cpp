#include "tabbar.hpp"

#include "../theme.hpp"

using namespace Views;
using ElementID = Theme::ElementID;

TabBar :: TabBar() noexcept
: UI::Window({0,0}, {1,0})
, _current(0)
{
}

void TabBar :: add_tab(std::string label) noexcept {
  _tabs.push_back(std::move(label));
  draw();
}

void TabBar :: current_index(int index) noexcept {
  if (index >= 0 && index < count())
    _current = index;
  draw();
}

int TabBar :: current_index() const noexcept {
  return _current;
}

int TabBar :: count() const noexcept {
  return int(_tabs.size());
}

void TabBar :: layout(UI::Pos pos, UI::Size size) {
  size.height = 1;
  resize(size);
  setPos(pos);
}

void TabBar :: draw() {
  erase();
  moveCursor(0, 0);

  int i = 0;
  for (const auto& label : _tabs) {
    if (i++ == _current)
      attrSet(Theme::get(ElementID::TABBAR_SELECTED));
    else
      attrSet(Theme::get(ElementID::TABBAR_UNSELECTED));

    *this << ' ' << label;
  }
}

bool TabBar :: handle_mouse(MEVENT& m) {
  if (m.y == pos.y /* + size.height - 1 */) {
    int label_x = 0;
    for (size_t index = 0; index < _tabs.size(); ++index) {
      label_x += int(_tabs[index].length()) + 1;
      if (m.x <= label_x) {
        if (index_changed)
          index_changed(index);
        break;
      }
    }

    return true;
  }

  return false;
}

#ifdef TEST_TABBAR
#include "../lib/test.hpp"
int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  TabBar b;
  b.layout({0,0}, {LINES,COLS});
  for (const auto s : {"Tab1", "Tab2", "Tab3"})
    b.add_tab(s);

  for (int colors : {0, 8, 256}) {
    if (colors > COLORS)
      break;
    Theme::load_theme_by_colors(colors);

    for (int i = 0; i < 3; ++i) {
      b.current_index(i);
      b.noutrefresh();
      doupdate();
      usleep(500 * 1000);
    }
  }

  TEST_END();
}
#endif
