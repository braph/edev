#ifndef BROWSER_HPP
#define BROWSER_HPP

#include "../database.hpp"
#include "../ui/container.hpp"
#include "../widgets/listwidget.hpp"

#include <vector>

namespace Views {

using namespace Database;

struct Item {
  enum type_t {
    ITEM_TRACK,  // Column rendered track
    ITEM_BACK,   // [..]
    ITEM_FOLDER, // [Some Artist Title] / [Some Album Title]
    ITEM_PATH,   // [Artist] / [Album]
  };

  union data_t {
    Tracks::Track track;   // For ITEM_TRACK and ITEM_FOLDER
    const ColumnID* path;  // For ITEM_PATH

    data_t(Tracks::Track t)   : track(t) {}
    data_t(const ColumnID* p) : path(p) {}
  };

  type_t type;
  data_t data;

  Item(type_t t, const ColumnID* path)  : type(t), data(path)  {}
  Item(type_t t, Tracks::Track track)   : type(t), data(track) {}
};

struct Browser : public ListWidget<std::vector<Item>> {
  Browser();

  bool handle_key(int key) override;
  void render(WINDOW*, int, int, const Item&, int, unsigned);
  void fill_list();

private:
  struct Filter {
    ColumnID column;
    Field    field;
  };

  const ColumnID*    _current_column;
  ColumnID           _current_column_display;
  std::vector<Filter> _filters;
  std::vector<Item>   _list;
};

} // namespace Views

#endif
