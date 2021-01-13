#include "playlist.hpp"

#include "mainwindow.hpp"
#include "../widgets/listwidget.hpp"
#include "../ui/colors.hpp"
#include "../config.hpp"
#include "../theme.hpp"
#include "../actions.hpp"
#include "../bindings.hpp"

#include <lib/algorithm.hpp> // clamp
#include <lib/string.hpp>

#include <cstring>

using namespace UI;
using namespace Views;

/* ============================================================================
 * TrackRenderer - display a track as a row with columns
 * ==========================================================================*/

void TrackRenderer :: operator()(
    WINDOW *win,
    int y,
    int width,
    const Database::Tracks::Track &item,
    int index,
    unsigned flags
) {
  unsigned int additional_attributes = 0;
  if (flags & ITEM_ACTIVE)       additional_attributes |= A_BOLD;
  if (flags & ITEM_UNDER_CURSOR) additional_attributes |= A_STANDOUT;

  int x = 0;

  // Substract the space that separates the columns
  width = width - int(m_columns.size()) + 1;
  // Sum of all relative widths
  int _100Percent = 0;
  for (const auto& column : m_columns) {
    if (column.relative)
      // The sum of all relative columns should be 100, but may be another value
      _100Percent += column.size;
    else
      // Remove the fixed size columns
      width -= column.size;
  }

  for (const auto& column : m_columns) {
    if (flags & ITEM_SELECTED)
      wattrset(win, colors.list_item_selection | additional_attributes);
    else
      wattrset(win, Colors::set(column.fg, column.bg) | additional_attributes);

    const char* value = Database::track_column_to_string(item, column.tag);
    size_t len = std::mbstowcs(NULL, value, 0);
    int colwidth;

    if (column.relative)
      colwidth = width * column.size / _100Percent;
    else
      colwidth = column.size;

    // Clear the column field with spaces
    mvwhline(win, y, x, ' ', colwidth + 1);

    if (column.justify == PlaylistColumnFormat::Justify::Right && int(len) < colwidth)
      mvwaddstr(win, y, x + colwidth - int(len), value);
    else // Justify::Left or no space to justify
      mvwaddnstr(win, y, x, value, colwidth);

    x += colwidth + 1;
  }
}

/* ============================================================================
 * Playlist
 * ==========================================================================*/

Playlist :: Playlist()
: _track_renderer(Config::playlist_columns)
{
  this->itemRenderer = _track_renderer;
  this->list(&this->playlist);
}

bool Playlist :: handle_key(int key) {
  bool search_reverse = false;

  if (! Bindings::playlist[key])
    return false;

  switch (Bindings::playlist[key]) {
  case Actions::TOP:       top();        break;
  case Actions::BOTTOM:    bottom();     break;
  case Actions::UP:        up();         break;
  case Actions::DOWN:      down();       break;
  case Actions::PAGE_UP:   page_up();    break;
  case Actions::PAGE_DOWN: page_down();  break;
  case Actions::SEARCH_UP:
     search_reverse = true; // fall-through
  case Actions::SEARCH_DOWN:
     mainwindow->readline("Search: ", [&, search_reverse](std::string line, bool) {
       _track_search.start_search(this->playlist,
         [=](const Database::Tracks::Track& track) {
            for (const auto& column : Config::playlist_columns)
              if (icontains(Database::track_column_to_string(track, column.tag), line))
                return true;
            return false;
        }, search_reverse);

       if (_track_search.next())
         cursor_index(_track_search.index());
     });
     break;

  case Actions::SEARCH_NEXT:
     if (_track_search.next())
       cursor_index(_track_search.index());
     break;

  case Actions::SEARCH_PREV:
     if (_track_search.prev())
       cursor_index(_track_search.index());
     break;

  default:
     Actions::call(Bindings::playlist[key]);
  }

  return true;
}

#ifdef TEST_PLAYLIST
#include <lib/test.hpp>

int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Config::init();
  load_theme_by_colors(COLORS, colors);
  Database::Database db;
  db.load(TEST_DB);
  assert(db.tracks.size() > 10);

  TrackRenderer renderer(Config::playlist_columns);
  auto tracks = db.get_tracks();

  testListItemRenderer(tracks, renderer);

  testListWidget(tracks, renderer);

  TEST_END();
}
#endif
