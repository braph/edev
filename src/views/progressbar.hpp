#ifndef VIEWS_PROGRESSBAR_HPP
#define VIEWS_PROGRESSBAR_HPP

#include "../ui/core.hpp"

#include <functional>

namespace Views {

class ProgressBar : public UI::Pad {
public:
  ProgressBar();

  void draw()                     override;
  void layout(UI::Pos, UI::Size)  override;
  bool handle_mouse(MEVENT&)      override;

  void percent(float)             noexcept;

  std::function<void(float)> percent_changed;
};

} // namespace Views

#endif
