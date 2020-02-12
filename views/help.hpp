#ifndef _HELP_HPP
#define _HELP_HPP

#include "../ui.hpp"

namespace Views {
  class Help : public UI::Pad {
  public:
    void draw();
    void layout(UI::Pos, UI::Size);
  };
}

#endif
