#ifndef UI_CONTAINER_HPP
#define UI_CONTAINER_HPP

#include "../ui.hpp"

#include <vector>

namespace UI {

class GenericContainer : public Widget {
public:
  GenericContainer()               noexcept;

  void draw()                      override;
  void noutrefresh()               override;
  bool handleKey(int)              override;
  bool handleMouse(MEVENT&)        override;
  WINDOW* getWINDOW()              const noexcept override;

  void    addWidget(Widget*);
  Widget* currentWidget()          const noexcept;
  void    currentWidget(Widget*)         noexcept;
  int     currentIndex()           const noexcept;
  void    currentIndex(int)              noexcept;
  int     indexOf(Widget*)         const noexcept;
  bool    empty()                  const noexcept;
  int     count()                  const noexcept;

protected:
  std::vector<Widget*> _widgets;
  int _current;
};

class VerticalContainer : public GenericContainer {
public:
  VerticalContainer()              noexcept;

  void layout(Pos, Size)           override;
};

class StackedContainer : public GenericContainer {
public:
  StackedContainer()               noexcept;

  void draw()                      override;
  void noutrefresh()               override;
  void layout(Pos, Size)           override;
  bool handleMouse(MEVENT&)        override;
};

} // namespace UI
#endif
