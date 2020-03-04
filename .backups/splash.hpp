// Splash with pads....
#ifndef _SPLASH_HPP
#define _SPLASH_HPP

#include "../ui.hpp"

namespace Views {
  
class Splash : public UI::Pad {
public:
  Splash();
  void draw();
  void layout(UI::Pos, UI::Size);
  void noutrefresh();
};

} // namespace Views

#endif
