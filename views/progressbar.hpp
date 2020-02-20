#ifndef _PROGRESSBAR_HPP
#define _PROGRESSBAR_HPP

#include "../ui.hpp"

#include <functional>

namespace Views {

class ProgressBar : public UI::Pad {
public:
  std::function<void(float)> percentChanged;
  bool handleClick(int, int, int);

  void setPercent(float);
  void layout(UI::Pos, UI::Size);
  void draw();
};

} // namespace Views

#endif
