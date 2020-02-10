#ifndef _CONTAINER_HPP
#define _CONTAINER_HPP

#include "../ui.hpp"
namespace UI {

class GenericContainer : public Widget {
protected:
  std::vector<Widget*> widgets;
  int active;
public:
  GenericContainer();
  void draw();
  void refresh();
  void add(Widget*);
  WINDOW *active_win();
};

class VerticalContainer : public GenericContainer {
public:
  void layout(Pos, Size);
};

}
#endif
