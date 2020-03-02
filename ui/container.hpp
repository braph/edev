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
  bool handleKey(int);
  bool handleMouse(MEVENT&);

  WINDOW* active_win() const;
  Widget* currentWidget() const;
  int     indexOf(Widget*) const;
  int     currentIndex() const;
  void    addWidget(Widget*);
  void    setCurrentIndex(int);
  bool    empty() const;
  int     count() const;

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
  void layout(Pos, Size);
  bool handleMouse(MEVENT&);
};

} // namespace UI
#endif
