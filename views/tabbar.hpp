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
  void layout(UI::Pos, UI::Size);
  void draw();
  void add(const std::string&);
  void select(unsigned int);
private:
  std::vector<std::string> tabs;
  unsigned int current;
};

} // namespace Views
#endif
