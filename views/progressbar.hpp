#ifndef VIEWS_PROGRESSBAR_HPP
#define VIEWS_PROGRESSBAR_HPP

#include "../ui.hpp"

#include <functional>

namespace Views {

class ProgressBar : public UI::Pad {
public:
  ProgressBar();

  void draw()                         override;
  void layout(UI::Pos, UI::Size)      override;
  bool handleMouse(MEVENT&)           override;

  void setPercent(float)              noexcept;

  std::function<void(float)> percentChanged;
};

} // namespace Views

#endif
