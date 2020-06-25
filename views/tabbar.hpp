#ifndef VIEWS_TABBAR_HPP
#define VIEWS_TABBAR_HPP

#include "../ui.hpp"

#include <string>
#include <vector>
#include <functional>

namespace Views {

class TabBar : public UI::Window {
public:
  TabBar()                            noexcept;

  void  draw()                        override;
  void  layout(UI::Pos, UI::Size)     override;
  bool  handle_mouse(MEVENT&)         override;

  void  add_tab(std::string)          noexcept;
  void  current_index(int)            noexcept;
  int   current_index()         const noexcept;
  int   count()                 const noexcept;

  std::function<void(int)> index_changed;

private:
  std::vector<std::string> _tabs;
  int _current;
};

} // namespace Views

#endif
