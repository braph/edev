#ifndef _CONTAINER_HPP
#define _CONTAINER_HPP

#include "../ui.hpp"
#include <vector>
namespace UI {

class GenericContainer : public Widget {
protected:
  std::vector<Widget*> widgets;
  int active;
public:
  GenericContainer();
  void draw();
  void noutrefresh();
  void add(Widget*);
  WINDOW *active_win();
};

class VerticalContainer : public GenericContainer {
public:
  VerticalContainer();
  void layout(Pos, Size);
};

class StackedContainer : public GenericContainer {
public:
  StackedContainer();
  void layout(Pos, Size);
};

}
#endif
