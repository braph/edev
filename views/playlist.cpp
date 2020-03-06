#include "playlist.hpp"

#include "../widgets/listwidget.hpp"
#include "mainwindow.hpp"
#include "rm_trackstr.cpp" //XXX
#include "../config.hpp"
#include "../colors.hpp"
#include "../theme.hpp"
#include "../database.hpp"
#include "../common.hpp"

#include <cstring>

using namespace UI;
using namespace Views;

/* ============================================================================
 * TrackRenderer - display a track as a row with columns
 * ==========================================================================*/

void TrackRenderer :: operator()(
    WINDOW *_win,
    int width,
    const Database::Tracks::Track &item,
    int index,
    bool cursor,
    bool active /* selection */
) {
  unsigned int additional_attributes = 0;
  if (active) additional_attributes |= A_BOLD;
  if (cursor) additional_attributes |= A_STANDOUT;
  int selection = 0; // XXX: this is a parameter

  int y = getcury(_win);
  int x = 0;

  // Substract the space that separates the columns
  width = width - int(m_columns.size()) + 1;
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
      wattrset(_win, Theme::get(Theme::LIST_ITEM_SELECTION) | additional_attributes);
    else
      wattrset(_win, Colors::set(column.fg, column.bg, additional_attributes));

    size_t len;
    wchar_t* value = toWideString(trackField(item, column.tag), &len);
    int colwidth;

    if (column.relative)
      colwidth = width * column.size / _100Percent;
    else
      colwidth = column.size;

    // Clear the column field with spaces
    mvwhline(_win, y, x, ' ', colwidth + 1);

    if (column.justify == PlaylistColumnFormat::Left)
      mvwaddnwstr(_win, y, x, value, colwidth);
    else if (column.justify == PlaylistColumnFormat::Right) {
      if (int(len) < colwidth)
        mvwaddwstr(_win, y, x + colwidth - int(len), value); // TODO
      else
        mvwaddnwstr(_win, y, x, value, colwidth);
    }

    x += colwidth + 1;
  }
}

/* ============================================================================
 * Playlist
 * ==========================================================================*/

Playlist :: Playlist(Actions& actions, Views::MainWindow& mainwindow)
: actions(actions)
, mainwindow(mainwindow)
, trackRenderer(Config::playlist_columns)
{
  this->itemRenderer = trackRenderer;
  this->attachList(&this->playlist);
}

bool Playlist :: handleKey(int key) {
  if (Bindings::playlist[key]) {
    switch (Bindings::playlist[key]) {
    case Actions::TOP:       top();        break;
    case Actions::BOTTOM:    bottom();     break;
    case Actions::UP:        up();         break;
    case Actions::DOWN:      down();       break;
    case Actions::PAGE_UP:   page_up();    break;
    case Actions::PAGE_DOWN: page_down();  break;
    case Actions::SEARCH:
       mainwindow.readline("Search: ", [&](const std::string& line, bool notEOF) {
           trackSearch.startSearch(this->playlist,
             [=](const Database::Tracks::Track& track) {
                for (const auto &column : Config::playlist_columns)
                  if (boost::algorithm::icontains(trackField(track, column.tag), line))
                    return true;
                return false;
            });

           if (trackSearch.next())
             cursorIndex(trackSearch.index());
         });
       break;
    case Actions::SEARCH_NEXT: if (trackSearch.next())
                                 cursorIndex(trackSearch.index());
                               break;
    case Actions::SEARCH_PREV: if (trackSearch.prev())
                                 cursorIndex(trackSearch.index());
                               break;

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
  assert(db.tracks.size() > 10);

  TrackRenderer renderer(Config::playlist_columns);
  auto tracks = db.getTracks();

  testListItemRenderer(tracks, renderer);

  testListWidget(tracks, renderer);

  TEST_END();
}
#endif
