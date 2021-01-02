#include "browser.hpp"

#include "../widgets/listwidget.hpp"
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

bool BrowserFilterSelector :: handle_key(int key) {
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

bool Browser :: handle_key(int key) {
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
  auto tracks = db.get_tracks();

  testListItemRenderer(tracks, renderer);

  testListWidget(tracks, renderer);

  TEST_END();
}
#endif
