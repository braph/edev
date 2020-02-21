#include "playlist.hpp"

#include "../widgets/listwidget.hpp"

#include "../config.hpp"
#include "../colors.hpp"
#include "../theme.hpp"
#include "../database.hpp"

#include "rm_trackstr.cpp"

#include <cstring>

using namespace UI;
using namespace Views;

void TrackRenderer :: render(WINDOW *win, const Database::Tracks::Track &item, int index, bool cursor, bool active /* selection */) {
  int additional_attributes = 0;
  if (active) additional_attributes |= A_BOLD;
  if (cursor) additional_attributes |= A_STANDOUT;
  int selection = 0; // XXX: this is a parameter

  int y = getcury(win);
  int x = 0;

  // Substract the space that separates the columns
  int width = m_width - m_columns.size() + 1;
  // Sum of all relative widths
  int _100Percent = 0;
  for (const auto &column : m_columns) {
    if (column.relative)
      // The sum of all relative columns should be 100, but may be another value
      _100Percent += column.size;
    else
      // Remove the fixed size columns
      width -= column.size;
  }

  for (const auto &column : m_columns) {
    if (selection)
      wattrset(win, Theme::get(Theme::LIST_ITEM_SELECTION) | additional_attributes);
    else
      wattrset(win, Colors::set(column.fg, column.bg, additional_attributes));

    size_t len;
    wchar_t* value = toWideString(trackField(item, column.tag), &len);
    size_t colwidth;

    if (column.relative)
      colwidth = width * column.size / _100Percent;
    else
      colwidth = column.size;

    // Clear the column field with spaces
    mvwhline(win, y, x, ' ', colwidth + 1);

    if (column.justify == PlaylistColumnFormat::Left)
      mvwaddnwstr(win, y, x, value, colwidth);
    else if (column.justify == PlaylistColumnFormat::Right) {
      if (len < colwidth)
        mvwaddwstr(win, y, x + colwidth - len, value); // TODO
      else
        mvwaddnwstr(win, y, x, value, colwidth);
    }

    x += colwidth+1;
  }
}

#if TEST_PLAYLIST
#include "../test.hpp"

int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Config::init();
  Theme::loadTheme(COLORS);
  Database db;
  db.load(TEST_DB);
  assert(db.tracks.size() > 10);

  TrackRenderer renderer(Config::playlist_columns);
  auto tracks = db.getTracks();

  testListItemRenderer(tracks, renderer);

  renderer.setWidth(COLS);
  testListWidget(tracks, renderer);

  TEST_END();
}
#endif
