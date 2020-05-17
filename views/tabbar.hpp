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
  bool  handleMouse(MEVENT&)          override;

  void  addTab(std::string)           noexcept;
  void  setCurrentIndex(int)          noexcept;
  int   currentIndex()          const noexcept;
  int   count()                 const noexcept;

  std::function<void(int)> indexChanged;

private:
  std::vector<std::string> _tabs;
  int _current;
};

} // namespace Views

#endif
