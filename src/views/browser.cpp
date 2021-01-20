#include "browser.hpp"

#include "../config.hpp"
#include "../actions.hpp"
#include "../ui/colors.hpp"
#include "../theme.hpp"
#include "../actions.hpp"
#include "../bindings.hpp"

#include <lib/iterator/iterator_pair.hpp>

namespace Views {

using namespace Database;

static const char* column_id_to_string(ColumnID id) {
  switch (id) {
  case static_cast<ColumnID>(STYLE_NAME):            return "style";
  case static_cast<ColumnID>(ALBUM_TITLE):           return "album";
  case static_cast<ColumnID>(ALBUM_ARTIST):          return "album_artist";
  case static_cast<ColumnID>(ALBUM_DESCRIPTION):     return "description";
  case static_cast<ColumnID>(ALBUM_DATE):            return "date";
  case static_cast<ColumnID>(ALBUM_RATING):          return "rating";
  case static_cast<ColumnID>(ALBUM_VOTES):           return "votes";
  case static_cast<ColumnID>(ALBUM_DOWNLOAD_COUNT):  return "downloads";
  case static_cast<ColumnID>(ALBUM_DAY):             return "day";
  case static_cast<ColumnID>(ALBUM_MONTH):           return "month";
  case static_cast<ColumnID>(ALBUM_YEAR):            return "year";
  case static_cast<ColumnID>(TRACK_TITLE):           return "title";
  case static_cast<ColumnID>(TRACK_ARTIST):          return "artist";
  case static_cast<ColumnID>(TRACK_REMIX):           return "remix";
  case static_cast<ColumnID>(TRACK_NUMBER):          return "number";
  case static_cast<ColumnID>(TRACK_BPM):             return "bpm";
  case static_cast<ColumnID>(ALBUM_STYLES):          return "styles";
  default:                                           return "unknown";
  }
}

static const ColumnID paths[3][3] = {
  {  column_cast(ALBUM_ARTIST),  column_cast(ALBUM_TITLE),  column_cast(COLUMN_NONE)  },
  {  column_cast(STYLE_NAME),    column_cast(ALBUM_TITLE),  column_cast(COLUMN_NONE)  },
  {  column_cast(ALBUM_YEAR),    column_cast(ALBUM_TITLE),  column_cast(COLUMN_NONE)  },
};

Window0 :: Window0(Browser* browser, const Window0* parent, const ColumnID* current_column)
  : _browser(browser)
  , _parent(parent)
  , _current_column(current_column)
{
  itemRenderer = [this]
    (WINDOW* win, int y, int width, const Item& item, int i, unsigned flags)
    { render(win, y, width, item, i, flags); };
  list(&_list);
  fill_list();
}

Window0 :: Window0(Browser* browser, const Window0* parent, const ColumnID* current_column, Field filter)
  : _browser(browser)
  , _parent(parent)
  , _current_column(current_column)
  , _current_filter(filter)
{
  itemRenderer = [this]
    (WINDOW* win, int y, int width, const Item& item, int i, unsigned flags)
    { render(win, y, width, item, i, flags); };
  list(&_list);
  fill_list();
}

void Window0 :: render(
    WINDOW *win,
    int y,
    int width,
    const Item& item,
    int index,
    unsigned flags
) {
  const char* text;
  const Tracks::Track track = item.data.track;
  switch (item.type) {
    case Item::ITEM_TRACK:   return Views::TrackRenderer(Config::playlist_columns)(win, y, width, track, index, flags);
    case Item::ITEM_BACK:    text = "[..]"; break;
    case Item::ITEM_FOLDER:  text = track_column_to_string(item.data.track, *_current_column); break;
    case Item::ITEM_PATH:    text = column_id_to_string(*item.data.path); break;
  }

  unsigned int attributes = 0;
  if (flags & ITEM_ACTIVE)       attributes |= A_BOLD;
  if (flags & ITEM_UNDER_CURSOR) attributes |= A_STANDOUT;
  if (flags & ITEM_SELECTED)     attributes |= colors.list_item_selection;
  attributes |= (index % 2) ? colors.list_item_odd : colors.list_item_even;
  wattrset(win, attributes);
  wprintw(win, "[%s]", text);
}

void Window0 :: fill_list() {
  _list.clear();

  if (! _parent /* This is our root window */) {
    _list.push_back(Item(Item::ITEM_PATH, paths[0]));
    _list.push_back(Item(Item::ITEM_PATH, paths[1]));
    _list.push_back(Item(Item::ITEM_PATH, paths[2]));
  }

  if (_current_column && *_current_column /* We list folders */) {
    assert(_parent);

    // TODO _current_filter

    ColumnID col = *_current_column;
    if (col == static_cast<ColumnID>(STYLE_NAME))
      col = static_cast<ColumnID>(ALBUM_STYLES);

    std::vector<Database::Field> having;
    for (auto item : _parent->_list)
      if (item.type == Item::ITEM_TRACK && having.end() == std::find(having.begin(), having.end(), item.data.track[col])) {
        _list.push_back(Item(Item::ITEM_FOLDER, item.data.track));
        having.push_back(item.data.track[col]);
      }
  }

  // List all the tracks
  if (_parent) {
    for (auto item : _parent->_list) {
      if (item.type == Item::ITEM_TRACK)
        _list.push_back(item);
    }
  }
  else {
    for (auto track : make_iterator_pair(database.tracks.begin(), database.tracks.end())) {
      _list.push_back(Item(Item::ITEM_TRACK, track));
    }
  }
}

bool Window0 :: handle_key(int key) {
  if (Bindings::playlist[key]) {
    switch (Bindings::playlist[key]) {
    case Actions::TOP:       top();        break;
    case Actions::BOTTOM:    bottom();     break;
    case Actions::UP:        up();         break;
    case Actions::DOWN:      down();       break;
    case Actions::PAGE_UP:   page_up();    break;
    case Actions::PAGE_DOWN: page_down();  break;
    case Actions::PLAYLIST_PLAY: { // TODO
      auto item = cursor_item();
      switch (item.type) {
      case Item::ITEM_PATH:
        _browser->add_widget(new Window0(_browser, this, item.data.path));
        _browser->current_index(_browser->count() - 1);
        return true;
      case Item::ITEM_BACK:
        _browser->pop_back();
        return true;
      case Item::ITEM_FOLDER:
        _browser->add_widget(new Window0(_browser, this, _current_column + 1, item.data.track[*_current_column]));
        _browser->current_index(_browser->count() - 1);
        return true;
      case Item::ITEM_TRACK:
      default:
        return true;
      }}
    default: Actions::call(Bindings::playlist[key]);
    }
    return true;
  }
  
  return false;
}

Browser :: Browser()
  : _root(this)
{
  add_widget(&_root);
}

} // namespace Views
