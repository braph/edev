#ifndef _BROWSER_HPP
#define _BROWSER_HPP

#include "../actions.hpp"
#include "../bindings.hpp"
#include "../database.hpp"
#include "../ui/container.hpp"
#include "../widgets/listwidget.hpp" // XXX

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
 * 1   Kino Oko             
 */

namespace Views {

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

class Browser : public ListWidget<std::vector<Database::Tracks::Track>> {
public:
  Browser(Actions&, Database&);
  std::vector<BrowserItem> items;
  bool handleKey(int);
private:
  Actions& actions;
  Database& database;
  BrowserItemRenderer trackRenderer;
};

} // namespace Views

#endif
