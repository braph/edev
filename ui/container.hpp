#ifndef _CONTAINER_HPP
#define _CONTAINER_HPP

#include "../ui.hpp"
#include <vector>
namespace UI {

class GenericContainer : public Widget {
public:
  GenericContainer();
  void draw();
  void noutrefresh();

  WINDOW* active_win();
  void addWidget(Widget*);
  Widget* currentWidget();
  int indexOf(Widget*);
  int currentIndex();
  void setCurrentIndex(int);
  size_t count();
  bool handleClick(int,int,int);
protected:
  std::vector<Widget*> _widgets;
  int _current;
};

class VerticalContainer : public GenericContainer {
public:
  VerticalContainer();
  void layout(Pos, Size);
};

class StackedContainer : public GenericContainer {
public:
  StackedContainer();
  void draw();
  void noutrefresh();
  bool handleClick(int,int,int);
  void layout(Pos, Size);
};

}
#endif
