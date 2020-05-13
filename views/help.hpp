#ifndef VIEWS_HELP_HPP
#define VIEWS_HELP_HPP

#include "../ui.hpp"

namespace Views {

class Help : public UI::Pad {
public:
  void draw();
  void layout(UI::Pos, UI::Size);
  bool handleKey(int);
};

} // namespace Views

#endif
