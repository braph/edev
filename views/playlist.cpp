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
    WINDOW *win,
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

  int y = getcury(win);
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
      wattrset(win, Theme::get(Theme::LIST_ITEM_SELECTION) | additional_attributes);
    else
      wattrset(win, Colors::set(column.fg, column.bg, additional_attributes));

    size_t len;
    wchar_t* value = toWideString(trackField(item, column.tag), &len);
    int colwidth;

    if (column.relative)
      colwidth = width * column.size / _100Percent;
    else
      colwidth = column.size;

    // Clear the column field with spaces
    mvwhline(win, y, x, ' ', colwidth + 1);

    if (column.justify == PlaylistColumnFormat::Left)
      mvwaddnwstr(win, y, x, value, colwidth);
    else if (column.justify == PlaylistColumnFormat::Right) {
      if (int(len) < colwidth)
        mvwaddwstr(win, y, x + colwidth - int(len), value); // TODO
      else
        mvwaddnwstr(win, y, x, value, colwidth);
    }

    x += colwidth + 1;
  }
}

/* ============================================================================
 * TrackSearch - Circular search through what is displayed by the TrackRenderer
 * ==========================================================================*/

TrackSearch :: TrackSearch(const PlaylistColumns& columns) : m_columns(columns), list(NULL) { }

bool TrackSearch :: next() {
  if (list) {
    for (index = clamp<size_t>(index + 1, 0, list->size() - 1); index < list->size(); ++index)
      if (indexMatchesCriteria())
        return true;

    index = std::numeric_limits<size_t>::max();
  }
  return false;
}

bool TrackSearch :: prev() {
  if (list) {
    for (index = clamp<size_t>(index - 1, 0, list->size() - 1); index; --index)
      if (indexMatchesCriteria())
        return true;

//    if (index == 0)
//      index = list->size();
  }
  return false;
}


bool TrackSearch :: indexMatchesCriteria() {
  for (const auto &column : m_columns)
    if (boost::algorithm::icontains(trackField((*list)[index], column.tag), query))
      return true;
  return false;
}

void TrackSearch :: startSearch(const std::string& q, std::vector<Database::Tracks::Track>* l) {
  list = l;
  query = q;
  index = std::numeric_limits<size_t>::max();
}

/* ============================================================================
 * Playlist
 * ==========================================================================*/

Playlist :: Playlist(Actions& actions, Views::MainWindow& mainwindow)
: actions(actions)
, mainwindow(mainwindow)
, trackRenderer(Config::playlist_columns)
, trackSearch(Config::playlist_columns)
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
    case Actions::SEARCH:    mainwindow.readline("Search: ", [&](const std::string& line, bool notEOF) {
                                 trackSearch.startSearch(line, &this->playlist);
                                 if (trackSearch.next())
                                   setSelected(trackSearch.getIndex());
                               });
                             break;
    case Actions::SEARCH_NEXT: if (trackSearch.next())
                                 setSelected(trackSearch.getIndex());
                               break;
    case Actions::SEARCH_PREV: if (trackSearch.prev())
                                 setSelected(trackSearch.getIndex());
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
