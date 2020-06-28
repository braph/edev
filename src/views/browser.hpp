#ifndef BROWSER_HPP
#define BROWSER_HPP

#include "../actions.hpp"
#include "../bindings.hpp"
#include "../database.hpp"
#include "../ui/container.hpp"
#include "../widgets/listwidget.hpp"

#include <vector>

/*
 * [Add new filter]
 * [Drop filter]
 * 1   Unknown Artist                     Kalila                               
 * 1   Mukti                              Magick Mother                        
 * 1   Unknown Artist                     Telepathik Nonsense                  
 * 1   SoulCraft                          Zen Spirit                           
 * 1   Annunaki                           T.O.U.C.H. Samadhi 001               
 * 1   Disco Hooligans                    Clear Skies                          
 * 1   Unknown Artist                     40 Full Moons                        
 * 1   Quantica                           Groove & Roots                       
 * 1   Knifestyle                         Rainsaw                              
 * 1   Kino Oko                                                         */


namespace Views {

  using TrackList = std::vector<Database::Tracks::Track>;

  class Filters {
  public:
    struct Filter {
      Database::ColumnID id;
      std::string value;
    };

    void apply(TrackList&);
    void addFilter(Database::ColumnID, const std::string& value);
    void popFilter();
  private:
    std::vector<Filter> _filters;
  };

} // namespace Views

namespace Views {

void Filters :: apply(TrackList& trackList) {
  for (const auto& filter : _filters) {
    std::stable_sort;
  }
}


// ============================================================================

class TagList : public UI::ListWidget<vector<const char*>> {
public:
  TagList();
  void bool handle_key(int);
private:
  std::vector<const char*> list;
};

TagList :: TagList() {
  items.clear();
  items.push_back("Cancel");
  for (auto id : Database::ColumnIDs)
    items.push_back(id.to_string());
}

bool TagList :: handle_key(int key) {
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
}

// ============================================================================

class ValueList : public UI::ListWidget<vector<const char*>> {
public:
  ValueList();
  void bool handle_key(int);
  void loadFoo();
private:
  std::vector<const char*> list;
  Database::ColumnID id;
};

void ValueList :: loadFoo(const std::vector<Database::Tracks::Track>& result) {

}

// ============================================================================

class TrackList : public UI::ListWidget<> {
};

// ============================================================================

class Browser : public UI::StackedContainer {
public:
  Browser();

private:
  Database& _database;
  TagList   _tagListWidget;
  ValueList _valueListWidget;
  TrackList _trackListWidget;
};

Browser :: Browser()
: _tagListWidget()
, _valueListWidget()
, _trackListWidget()
, _database(database)
{
  add_widget(&_tagListWidget);
  add_widget(&_valueListWidget);
  add_widget(&_trackListWidget);

  _tagListWidget.onKeyPress = [&](int key) {

  };

  _tagListWidget.onSelected = [&](){
    if (_tagListWidget.index() == 0) {
      setCurrentWidget(_trackListWidget);
    } else {
      _valueListWidget.load(_trackList, _tagListWidget.item().tagID);
      setCurrentWidget(_valueListWidget);
    }
  };

  _valueListWidget.onIndexSelected = [&]() {
    _filters.addFilter(_valueListWidget.tagID, _valueListWidget.item().value);
    setCurrentWidget(_trackList);
  };
}

// ============================================================================


class Browser : public ListWidget<std::vector<Database::Tracks::Track>> {
public:
  Browser(Actions&, Database&);
  std::vector<BrowserItem> items;
  bool handle_key(int);
private:
  Actions& actions;
  Database& database;
  BrowserItemRenderer trackRenderer;
};

} // namespace Views

struct BrowserItem {
  enum Type {
    Track,
    FilterAdd,
    FilterRemove,
    FilterSelectTag,
    FilterSelectTagValue,
    Cancel
  } type;

  struct _FilterSelectTag {
    Database::ColumnID id;
    const char* name;
  };

  struct _FilterSelectTagValue {
    Database::ColumnID id;
    const char* value;
  };

  union {
    Database::Tracks::Track track;
    _FilterSelectTag filterSelectTag;
    _FilterSelectTagValue filterSelectTagValue;
  };
};

class BrowserItemRenderer {
public:
  BrowserItemRenderer(const PlaylistColumns& columns) : m_columns(columns) { }
  void operator()(WINDOW*, int width, const Database::Tracks::Track&, int, bool, bool); // marked, selection
private:
  const PlaylistColumns& m_columns;
};
#endif
