#include "listwidget.hpp"

/* Only testing here */

#if TEST_LISTWIDGET
#include "../test.hpp"
#include <string>
#include <vector>

int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  ListItemRenderer<std::string> renderer(COLS);

  std::vector<std::string> testData = {
    "Hello",
    "This is a test string",
    "Foo Bar", "Muhahaha"
  };
  for (int i = 0; i < 50; ++i)
    testData.push_back(std::to_string(i));

  testListItemRenderer(testData, renderer);
  testListWidget(testData, renderer);

  TEST_END();
}
#endif
