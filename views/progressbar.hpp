#ifndef VIEWS_PROGRESSBAR_HPP
#define VIEWS_PROGRESSBAR_HPP

#include "../ui.hpp"

#include <functional>

namespace Views {

class ProgressBar : public UI::Pad {
public:
  ProgressBar();

  void draw();
  void layout(UI::Pos, UI::Size);

  void setPercent(float);

  std::function<void(float)> percentChanged;
  bool handleMouse(MEVENT&);
};

} // namespace Views

#endif
