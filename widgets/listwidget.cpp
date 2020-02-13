#include "listwidget.hpp"

// Since the ListWidget is implemented as a template, this file is only
// for testing purposes.

#if TEST_LISTWIDGET
#include "../test.hpp"
#include <string>
#include <vector>

void testListItemRenderer() {
  // Draw the whole screen with numbers, the 3rd line should be selected

  ListItemRenderer<int> intRenderer(COLS);
  for (int i = 0; i < LINES; ++i) {
    wmove(stdscr, i, 0);
    intRenderer.render(stdscr, i, i, i == 3, i == 2);
  }
  refresh();
}

int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  noecho();
  curs_set(0);
  //testListItemRenderer();
  ListItemRenderer<std::string> renderer(COLS);
  std::vector<std::string> testData = { "Hello", "This is a test string", "Foo Bar", "Muhahaha" };
  for (int i = 0; i < 50; ++i) testData.push_back(std::to_string(i));
  testListWidget(renderer, testData);

  TEST_END();
}
#endif
