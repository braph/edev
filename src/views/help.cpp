#include "help.hpp"

#include "../theme.hpp"
#include "../actions.hpp"
#include "../bindings.hpp"
#include "../ui/colors.hpp"

using namespace UI;
using namespace Views;

static const struct {
  const char* name;
  const Actions::ActionID* bindings;
}
widgets[] = {
  {"Global",   Bindings::global},
  {"Playlist", Bindings::playlist},
  {"Text Pad", Bindings::pad}
};

void Help :: layout(Pos pos, Size size) {
  this->pos  = pos;
  this->size = size;
  size.height = 200;
  resize(size);
  pad_minrow = 0;
  pad_mincol = 0;
}

void Help :: draw() {
  const int KEYS_START         = 3;
  const int COMMANDS_START     = 30;
  const int DESCRIPTIONS_START = 45;

  clear();
  int y = 0;

  for (const auto& widget : widgets) {
    attrset(colors.help_widget_name);
    addstr(++y, 1, widget.name);
    move(++y, KEYS_START);

    for (int id = 1; id < Actions::ACTIONID_COUNT; ++id) {
      int nkeys = 0;

      attrset(colors.help_key_name);
      for (int i = 0; i < KEY_MAX; ++i) {
        if (widget.bindings[i] == id) {
          if (nkeys++)
            *this << ", ";
          *this << keyname(i);
        }
      }

      if (nkeys) {
        attrset(colors.help_command_name);
        addstr(y, COMMANDS_START, Actions::to_string(Actions::ActionID(id)));
        move(++y, KEYS_START);
      }
    }
  }
}

bool Help :: handle_key(int n) {
  switch (Bindings::pad[n]) {
  case Actions::UP:         up();         return true;
  case Actions::DOWN:       down();       return true;
  case Actions::PAGE_UP:    page_up();    return true;
  case Actions::PAGE_DOWN:  page_down();  return true;
  case Actions::TOP:        top();        return true;
  case Actions::BOTTOM:     bottom();     return true;
  default:                                return false;
  }
}

#ifdef TEST_HELP
#include "../test.hpp"
int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Bindings::init();
  load_theme_by_colors(COLORS, colors);

  Widget *s = new Views::Help;
  s->layout({10,10}, {30,80});
  s->draw();
  s->noutrefresh();
  doupdate();
  wgetch(s->getWINDOW());

  TEST_END();
}
#endif
