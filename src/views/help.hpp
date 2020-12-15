#ifndef VIEWS_HELP_HPP
#define VIEWS_HELP_HPP

#include "../ui/core.hpp"

namespace Views {

class Help : public UI::Pad {
public:
  void draw()                         override;
  void layout(UI::Pos, UI::Size)      override;
  bool handle_key(int)                override;
};

} // namespace Views

#endif
