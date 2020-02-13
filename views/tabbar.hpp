#ifndef _TABBAR_CPP
#define _TABBAR_CPP

#include "../ui.hpp"

#include <vector>
#include <string>

namespace Views {

//typedef void (*changed)(int);

class TabBar : public UI::Window {
public:
  TabBar();
  void draw();
  void layout(UI::Pos, UI::Size);
  void addTab(const std::string&);
  int currentIndex();
  void setCurrentIndex(int);
  size_t count();
private:
  std::vector<std::string> _tabs;
  unsigned int _current;
};

} // namespace Views
#endif
