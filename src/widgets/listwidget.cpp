#include "listwidget.hpp"

#include <lib/test.hpp>
#include <string>
#include <vector>

/* ONLY TESTING HERE, NOT IMPLEMENTATION */

template<typename TContainer, typename TRender>
void testListWidget(
  TContainer& testData,
  TRender& render
) {
  ListWidget<TContainer> listWidget;
  listWidget.item_renderer = render;
  listWidget.list(&testData);
  listWidget.layout({0,0}, {LINES,COLS});
  listWidget.draw();
  listWidget.noutrefresh();
  doupdate();

  for (;;) {
    switch (wgetch(listWidget.getWINDOW())) {
      case 'k': listWidget.up();        break;
      case 'j': listWidget.down();      break;
      case KEY_PPAGE:
      case 'K': listWidget.page_up();   break;
      case KEY_NPAGE:
      case 'J': listWidget.page_down(); break;
      case 'g': listWidget.top();       break;
      case 'G': listWidget.bottom();    break;
      case 'l': listWidget.draw();      break;
      case 'q': return;
    }
    listWidget.noutrefresh();
    doupdate();
  }
}

template<typename TContainer, typename TRender>
void testListItemRenderer(TContainer& container, TRender& render) {
  int cursor = LINES / 2;

  for (int y = 0; y < container.size(); ++y) {
    if (y >= LINES)
      break;

    unsigned flags =
      (y == cursor ? ITEM_UNDER_CURSOR : 0u) |
      (y == 3      ? ITEM_ACTIVE       : 0u);

    wmove(stdscr, y, 0);
    render(stdscr, y, COLS, container[y], y, flags);
  }
  refresh();
  getch();
}

/* ============================================================================
 * ListItemRenderer - render an item in a list
 * ==========================================================================*/

template<typename TItem>
void testRender(WINDOW *win, int y, int width, const TItem &item, int index, unsigned flags) {
  // A primitive default renderer for testing purposes
  std::stringstream ss; ss << item;
  char marker = ' ';
  switch (flags) {
    case ITEM_UNDER_CURSOR:             marker = '>'; break;
    case ITEM_ACTIVE:                   marker = '+'; break;
    case ITEM_UNDER_CURSOR|ITEM_ACTIVE: marker = 'x'; break;
  }
  waddch(win, marker);
  waddnstr(win, ss.str().c_str(), width - 1);
}

int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  std::vector<std::string> testData = {
    "Hello",
    "This is a test string",
    "Foo Bar", "Muhahaha"
  };
  for (int i = 0; i < 50; ++i)
    testData.push_back(std::to_string(i));

  testListItemRenderer(testData, testRender<std::string>);
  testListWidget(testData, testRender<std::string>);

  TEST_END();
}
#endif
