#include "listwidget.hpp"

#if TEST_LISTWIDGET
#include "../test.hpp"
#include <string>

void testListItemRenderer() {
  // TODO: test descrip
  // Draw the whole screen with numbers, the 3rd line should be selected

  ListItemRenderer<int> intRenderer(COLS);
  for (int i = 0; i < LINES; ++i) {
    wmove(stdscr, i, 0);
    intRenderer.render(stdscr, i, i, i == 3, i == 2);
  }
  refresh();
}

template<typename ItemType>
void testListWidget(
    ListWidget<ItemType> listWidget,
    std::vector<ItemType> &testData)
{
  listWidget.setList(&testData);
  listWidget.layout(LINES, COLS);
  for (;;) {
    switch (getch()) {
      case 'k': listWidget.up();        break;
      case 'j': listWidget.down();      break;
      case 'K': listWidget.page_up();   break;
      case 'J': listWidget.page_down(); break;
      case 'g': listWidget.top();       break;
      case 'G': listWidget.bottom();    break;
      case 'l': listWidget.draw();      break;
      case 'q': return;
    }
    listWidget.refresh();
  }
}

int main() {
  TEST_BEGIN

  initscr();
  noecho();
  curs_set(0);
  start_color();
  use_default_colors();
  //Ektoplayer::Theme.load(256);

  testListItemRenderer();

  ListItemRenderer<std::string> renderer(COLS);
  ListWidget<std::string> listWidget(renderer);
  std::vector<std::string> testData = { "Hello", "This is a test string", "Foo Bar", "Muhahaha" };
  for (int i = 0; i < 50; ++i) testData.push_back(std::to_string(i));
  testListWidget<std::string>(listWidget, testData);

  TEST_END
}
#endif
