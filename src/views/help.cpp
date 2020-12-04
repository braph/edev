#include "help.hpp"

#include "../theme.hpp"
#include "../actions.hpp"
#include "../bindings.hpp"
#include "../ui/colors.hpp"

#define KEYS_START          3
#define COMMANDS_START      30
#define DESCRIPTIONS_START  45

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
  clear();

  int y = 0;

  for (const auto& widget : widgets) {
    attrSet(Theme::get(Theme::HELP_WIDGET_NAME));
    mvAddStr(++y, 1, widget.name);
    moveCursor(++y, KEYS_START);

    for (int id = 1; id < Actions::ACTIONID_ENUM_LAST; ++id) {
      int nkeys = 0;

      attrSet(Theme::get(Theme::HELP_KEY_NAME));
      for (int i = 0; i < KEY_MAX; ++i) {
        if (widget.bindings[i] == id) {
          if (nkeys++)
            *this << ", ";
          *this << keyname(i);
        }
      }

      if (nkeys) {
        attrSet(Theme::get(Theme::HELP_COMMAND_NAME));
        mvAddStr(y, COMMANDS_START, Actions::to_string(Actions::ActionID(id)));
        moveCursor(++y, KEYS_START);
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
  Theme::current = COLORS;

  Widget *s = new Views::Help;
  s->layout({10,10}, {30,80});
  s->draw();
  s->noutrefresh();
  doupdate();
  wgetch(s->getWINDOW());

  TEST_END();
}
#endif
