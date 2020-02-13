#ifndef _INFO_HPP
#define _INFO_HPP

#include "../ui.hpp"

namespace Views {

class Info : public UI::Pad {
public:
  void draw();
  void layout(UI::Pos, UI::Size);
};

} // namespace Views

#endif
