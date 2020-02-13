#include "../widgets/listwidget.hpp"

#include "../config.hpp"
#include "../colors.hpp"
#include "../database.hpp"

#include <cstring>

#include "rm_trackstr.cpp"

using namespace UI;

class TrackRenderer : public ListItemRenderer<SSMap> {
  private:
    const PlaylistColumns& m_columns;

  public:
    TrackRenderer(const PlaylistColumns& columns)
    : m_columns(columns)
    {
    }

    //void setFormat(format); -> wants layout
    virtual void render(WINDOW *win, const SSMap &item, int index, bool cursor, bool active); // marked, selection
};

// assert(column_format)
void TrackRenderer :: render(WINDOW *win, const SSMap &item, int index, bool cursor, bool active /* selection */) {
  int additional_attributes = 0;
  if (active) additional_attributes |= A_BOLD;
  if (cursor) additional_attributes |= A_STANDOUT;
  int selection = 0; // XXX: this is a parameter

  /*
  if (selection)
    color = UI::Colors.get("list.item_selection");
  else if (index % 2 == 0)
    color = UI::Colors.get("list.item_even");
  else
    color = UI::Colors.get("list.item_odd");

  scr.attrset(color | additional_attributes)
  scr.addstr("[#{item}]".ljust(@width))
  */
  // ---- SNAP
 
  int y = getcury(win);
  int x = 0; // lef_pad;

  // Substract the spaces that separates column
  int width = m_width - m_columns.size() - 1;
  // Sum of all relative widths
  int _100Percent = 0;
 for (const auto &column : m_columns) {
   if (column.relative)
     _100Percent += column.size; // The sum of all rel's shall be 100, but may be another value
   else
     width -= column.size; // Remove the fixed sizes
 }

 for (const auto &column : m_columns) {
   if (selection)
     wattrset(win, Colors::get("list.item_selection") | additional_attributes);
   else
     wattrset(win, Colors::set("", column.fg, column.bg, 0) | additional_attributes);

   const char* value = item.at(column.tag).c_str(); // RUBY %.2d
   int len = strlen(value);
   int colwidth;

   if (column.relative)
     colwidth = width * column.size / _100Percent;
   else
     colwidth = column.size;

   if (column.justify == PlaylistColumnFormat::Left)
     mvwaddnstr(win, y, x, value, colwidth);
   else if (column.justify == PlaylistColumnFormat::Right) {
     if (len < colwidth) {
       mvwhline(win, y, x, colwidth - len, ' '); // TODO
       waddnstr(win, value, len); // TODO
     } else {
       mvwaddnstr(win, y, x, value, colwidth);
     }
   }

   // if not last column
   waddch(win, ' ');
   x += colwidth;
 }
}

#if TEST_PLAYLIST
#include <unistd.h>
#include <string>
#include <iostream>

void testTrackRenderer(const SSMap& track, const PlaylistColumns& columns) {
  TrackRenderer tr(columns);
  tr.setWidth(COLS);

  int y = -1;
  wmove(stdscr, ++y, 0);  tr.render(stdscr, track, 0, false, false);
  wmove(stdscr, ++y, 0);  tr.render(stdscr, track, 0, true,  false);
  wmove(stdscr, ++y, 0);  tr.render(stdscr, track, 0, false, true); 
  wmove(stdscr, ++y, 0);  tr.render(stdscr, track, 0, true,  true);
  refresh();
}

int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  noecho();
  curs_set(0);
  Config::init();
  //Ektoplayer::Theme.load(256);

  testTrackRenderer(testData[0], Config::playlist_columns);

  /*
  ListItemRenderer<std::string> renderer(COLS);
  ListWidget<std::string> listWidget(renderer);
  std::vector<std::string> testData = { "Hello", "This is a test string", "Foo Bar", "Muhahaha" };
  for (int i = 0; i < 50; ++i) testData.push_back(std::to_string(i));
  testListWidget<std::string>(listWidget, testData);
  */

  mvwaddstr(stdscr, LINES - 2, 0, "pause();");
  pause();

  TEST_END();
}
#endif
