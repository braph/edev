#ifndef _PROGRESSBAR_HPP
#define _PROGRESSBAR_HPP

#include "../ui.hpp"

namespace Views {

//typedef void (*clicked)(percent);

class ProgressBar : public UI::Pad {
public:
  void setPercent(float);
  void layout(UI::Pos, UI::Size);
  void draw();
};

} // namespace Views

#endif
