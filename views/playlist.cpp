#include "playlist.hpp"

#include "../widgets/listwidget.hpp"

#include "../config.hpp"
#include "../colors.hpp"
#include "../theme.hpp"
#include "../database.hpp"

#include <cstring>

#include "rm_trackstr.cpp"

using namespace UI;

class TrackRenderer : public ListItemRenderer<Database::Tracks::Track> {
  private:
    const PlaylistColumns& m_columns;

  public:
    TrackRenderer(const PlaylistColumns& columns)
    : m_columns(columns)
    {
    }

    //void setFormat(format); -> wants layout
    void render(WINDOW *win, const Database::Tracks::Track &item, int index, bool cursor, bool active); // marked, selection
};

void TrackRenderer :: render(WINDOW *win, const Database::Tracks::Track &item, int index, bool cursor, bool active /* selection */) {
  int additional_attributes = 0;
  if (active) additional_attributes |= A_BOLD;
  if (cursor) additional_attributes |= A_STANDOUT;
  int selection = 0; // XXX: this is a parameter

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
     wattrset(win, Theme::get(Theme::LIST_ITEM_SELECTION) | additional_attributes);
   else
     wattrset(win, Colors::set(column.fg, column.bg, 0) | additional_attributes);

   const char* value = trackField(item, column.tag); // RUBY %.2d
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
#include "../test.hpp"
#include <string>
#include <algorithm>

void testTrackRenderer(Database &db, const PlaylistColumns& columns) {
  TrackRenderer tr(columns);
  tr.setWidth(COLS);

  auto tracks = db.getTracks();
  int cursor = LINES / 2;

  for (int y = 0; y < std::max(LINES, 10); ++y) {
    wmove(stdscr, y, 0);
    tr.render(stdscr, tracks[y], y, y == 3, y == cursor);
  }
  refresh();
  getch();
}

int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  noecho();
  curs_set(0);
  Config::init();
  Theme::loadTheme(COLORS);
  Database db;
  db.load(TEST_DB);
  assert(db.tracks.size() > 10);

  testTrackRenderer(db, Config::playlist_columns);

  TrackRenderer tr(Config::playlist_columns);
  tr.setWidth(COLS);
  auto tracks = db.getTracks();
  testListWidget(tr, tracks);

  /*
  ListItemRenderer<std::string> renderer(COLS);
  ListWidget<std::string> listWidget(renderer);
  std::vector<std::string> testData = { "Hello", "This is a test string", "Foo Bar", "Muhahaha" };
  for (int i = 0; i < 50; ++i) testData.push_back(std::to_string(i));
  testListWidget<std::string>(listWidget, testData);
  */

  TEST_END();
}
#endif
