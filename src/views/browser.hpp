#ifndef BROWSER_HPP
#define BROWSER_HPP

#include "../database.hpp"
#include "../ui/container.hpp"
#include "../widgets/listwidget.hpp"

#include <vector>

namespace Views {

using namespace Database;

struct Item {
  enum type {
    ITEM_TRACK,  // Column rendered track
    ITEM_BACK,   // [..]
    ITEM_FOLDER, // [Some Artist Title] / [Some Album Title]
    ITEM_PATH,   // [Artist] / [Album]
  };

  union data {
    Tracks::Track track; // For ITEM_TRACK and ITEM_FOLDER
    const ColumnID* path;          // For ITEM_PATH

    data(Tracks::Track t) : track(t) {}
    data(const ColumnID* p)         : path(p) {}
  };

  type type;
  data data;

  Item(enum type t, const ColumnID* path)  : type(t), data(path)  {}
  Item(enum type t, Tracks::Track track)   : type(t), data(track) {}
};

struct Window0 : public ListWidget<std::vector<Item>> {
  Window0(Window0* = NULL, ColumnID* = NULL);
  bool handle_key(int key) override;
  void render(WINDOW*, int, int, const Item&, int, unsigned);
  void fill_list();
private:
  Window0*   _parent;
  ColumnID* _current_column;
  int       _current_filter_id;
  std::vector<Item> _list;
  std::vector<Item>::const_iterator _tracks_begin;
  std::vector<Item>::const_iterator _tracks_end;
};

struct Browser : public UI::StackedContainer {
  Browser();
private:
  Window0 root;
};

} // namespace Views

#endif
