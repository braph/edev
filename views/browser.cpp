#include "browser.hpp"

#include "../widgets/listwidget.hpp"
#include "rm_trackstr.cpp"
#include "../config.hpp"
#include "../colors.hpp"
#include "../theme.hpp"
#include "../database.hpp"

#include <cstring>

using namespace UI;
using namespace Views;

/* ============================================================================
 * BrowserItemRenderer - display a track as a row with columns
 * ==========================================================================*/

void BrowserItemRenderer :: operator()(
    WINDOW *win,
    int width,
    const BrowserItem &item,
    int index,
    bool cursor,
    bool active /* selection */
) {
  int additional_attributes = 0;
  if (active) additional_attributes |= A_BOLD;
  if (cursor) additional_attributes |= A_STANDOUT;
  int selection = 0; // XXX: this is a parameter

  int y = getcury(win);
  int x = 0;

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

/* ============================================================================
 * Playlist
 * ==========================================================================*/


class BrowserFilterSelector : public ListWidget<const char*> {
  BrowserFilterSelector(Browser&);
};

BrowserFilterSelector :: BrowserFilterSelector() {
  this->attachList(&this->items);
  this->itemRenderer = []() { };

  items.clear();
  items.push_back("Cancel");
  for (auto id : Database::ColumnIDs)
    items.push_back(id.to_string());
}

bool BrowserFilterSelector :: handleKey(int key) {
  browser.addFilter("????");
}


Browser :: Browser(Actions& actions, Database& db)
: actions(actions), database(db), browserItemRenderer(Config::playlist_columns)
{
  this->itemRenderer = browserItemRenderer;
  this->attachList(&this->items);
}

void Browser :: load() {
  items.clear();

}

bool Browser :: handleKey(int key) {
  if (currentListWidget == normal && type == addFilter) {

  }

  if (Bindings::playlist[key]) {
    switch (Bindings::playlist[key]) {
    case Actions::TOP:       top();        break;
    case Actions::BOTTOM:    bottom();     break;
    case Actions::UP:        up();         break;
    case Actions::DOWN:      down();       break;
    case Actions::PAGE_UP:   page_up();    break;
    case Actions::PAGE_DOWN: page_down();  break;
    default: actions.call(Bindings::playlist[key]);
    }
    return true;
  }

  return false;
}

#ifdef TEST_PLAYLIST
#include "../test.hpp"

int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Config::init();
  Theme::loadTheme(COLORS);
  Database db;
  db.load(TEST_DB);
  //db.load(Config::database_file);
  assert(db.tracks.size() > 10);

  BrowserItemRenderer renderer(Config::playlist_columns);
  auto tracks = db.getTracks();

  testListItemRenderer(tracks, renderer);

  testListWidget(tracks, renderer);

  TEST_END();
}
#endif
