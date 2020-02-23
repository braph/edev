#include "listwidget.hpp"

/* Only testing here */

/* ============================================================================
 * ListItemRenderer - render an item in a list
 * ==========================================================================*/

template<typename TItem>
void testRender(WINDOW *win, int width, const TItem &item, int index, bool cursor, bool active) { // marked, selection
  // A primitive default renderer for testing purposes
  std::stringstream ss; ss << item;
  char marker = ' ';
  if (cursor && active) marker = 'X';
  else if (cursor)      marker = '>';
  else if (active)      marker = 'x';
  waddch(win, marker);
  waddnstr(win, ss.str().c_str(), width - 1);
}

#if TEST_LISTWIDGET
#include "../test.hpp"
#include <string>
#include <vector>

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
