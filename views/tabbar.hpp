#ifndef VIEWS_TABBAR_CPP
#define VIEWS_TABBAR_CPP

#include "../ui.hpp"

#include <vector>
#include <string>
#include <functional>

namespace Views {

class TabBar : public UI::Window {
public:
  TabBar();
  void draw();
  void layout(UI::Pos, UI::Size);
  bool handleMouse(MEVENT&);

  void   addTab(std::string);
  void   setCurrentIndex(int);
  int    currentIndex() const;
  int    count() const;
  std::function<void(int)> indexChanged;
private:
  std::vector<std::string> _tabs;
  int _current;
};

} // namespace Views
#endif
