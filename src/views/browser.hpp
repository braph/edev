#ifndef BROWSER_HPP
#define BROWSER_HPP

#include "../database.hpp"
#include "../ui/container.hpp"
#include "../widgets/listwidget.hpp"

#include <vector>

namespace Views {

using namespace Database;

struct Browser;

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

struct Window0 : public ListWidget<std::vector<Item>> {
  Window0(Browser*, const Window0* = NULL, const ColumnID* = NULL);
  Window0(Browser*, const Window0*, const ColumnID*, Field);

  bool handle_key(int key) override;
  void render(WINDOW*, int, int, const Item&, int, unsigned);
  void fill_list();

private:
  Browser*        _browser;
  const Window0*  _parent;
  const ColumnID* _current_column;
  Field           _current_filter;
  std::vector<Item> _list;
};

struct Browser : public UI::StackedContainer {
  Browser();
private:
  Window0 _root;
};

} // namespace Views

#endif
