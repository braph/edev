#ifndef VIEWS_HELP_HPP
#define VIEWS_HELP_HPP

#include "../ui.hpp"

namespace Views {

class Help : public UI::Pad {
public:
  void draw()                         override;
  void layout(UI::Pos, UI::Size)      override;
  bool handleKey(int)                 override;
};

} // namespace Views

#endif
