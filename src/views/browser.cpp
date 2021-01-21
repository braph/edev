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

Browser :: Browser()
  : _current_column(NULL)
{
  itemRenderer = [this]
    (WINDOW* win, int y, int width, const Item& item, int i, unsigned flags)
    { render(win, y, width, item, i, flags); };
  list(&_list);
  fill_list();
}

void Browser :: render(
    WINDOW *win,
    int y,
    int width,
    const Item& item,
    int index,
    unsigned flags
) {
  const char* text = "..";
  switch (item.type) {
    case Item::ITEM_TRACK:   Views::TrackRenderer(Config::playlist_columns)(win, y, width, item.data.track, index, flags); return;
    case Item::ITEM_BACK:    break;
    case Item::ITEM_FOLDER:  text = track_column_to_string(item.data.track, _current_column_display); break;
    case Item::ITEM_PATH:    text = column_id_to_string(*item.data.path); break;
  }

  attr_t attributes = (index % 2) ? colors.list_item_odd : colors.list_item_even;
  if (flags & ITEM_ACTIVE)       attributes |= A_BOLD;
  if (flags & ITEM_UNDER_CURSOR) attributes |= A_STANDOUT;
  if (flags & ITEM_SELECTED)     attributes |= colors.list_item_selection;
  wattrset(win, attributes);
  wprintw(win, "[%s]", text);
}

void Browser :: fill_list() {
  auto is_accepted = [this](const Database::Tracks::Track& track) {
    for (const auto& filter : _filters)
      if (! (track[filter.column] == filter.field))
        return false;
    return true;
  };

  _list.clear();
  _list.shrink_to_fit(); // TODO

  if (!_current_column /* Root window */) {
    _list.push_back(Item(Item::ITEM_PATH, paths[0]));
    _list.push_back(Item(Item::ITEM_PATH, paths[1]));
    _list.push_back(Item(Item::ITEM_PATH, paths[2]));
    _list.push_back(Item(Item::ITEM_PATH, &paths[0][1]));
  } else
    _list.push_back(Item(Item::ITEM_BACK, NULL));

  // Folders
  if (_current_column && *_current_column != COLUMN_NONE) {
    for (auto track : database.tracks) {
      auto folder_is_in_list = [&](const Item& item){
        return item.type == Item::ITEM_FOLDER && item.data.track[*_current_column] == track[*_current_column];
      };
      // TODO: styles exception

      if (is_accepted(track) && _list.end() == std::find_if(_list.begin(), _list.end(), folder_is_in_list))
        _list.push_back(Item(Item::ITEM_FOLDER, track));
    }
  }

  // List all the tracks
  for (auto track : database.tracks)
    if (is_accepted(track))
      _list.push_back(Item(Item::ITEM_TRACK, track));

  for (auto i : _list)
    if (i.type == Item::ITEM_TRACK)
      if (i.data.track.id == 0)
        throw 0;
}

bool Browser :: handle_key(int key) {
  auto update_column_id = [this](){
    if (_current_column)
      _current_column_display = *_current_column;
    if (_current_column_display == static_cast<ColumnID>(STYLE_NAME))
      _current_column_display = static_cast<ColumnID>(ALBUM_STYLES);
  };

  if (Bindings::playlist[key]) {
    switch (Bindings::playlist[key]) {
    case Actions::TOP:       top();        break;
    case Actions::BOTTOM:    bottom();     break;
    case Actions::UP:        up();         break;
    case Actions::DOWN:      down();       break;
    case Actions::PAGE_UP:   page_up();    break;
    case Actions::PAGE_DOWN: page_down();  break;
    case Actions::PLAYLIST_PLAY: {
      auto item = cursor_item();
      switch (item.type) {
      case Item::ITEM_PATH:
        _current_column = item.data.path;
        update_column_id();
        fill_list();
        return true;
      case Item::ITEM_BACK:
        if (_filters.size())
          _filters.pop_back();
        _current_column--;
        update_column_id();
        fill_list();
        return true;
      case Item::ITEM_FOLDER:
        _filters.push_back(Filter{*_current_column, item.data.track[*_current_column]});
        ++_current_column;
        update_column_id();
        fill_list();
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

} // namespace Views
