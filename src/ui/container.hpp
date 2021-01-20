#ifndef UI_CONTAINER_HPP
#define UI_CONTAINER_HPP

#include "core.hpp"

#include <vector>

namespace UI {

class GenericContainer : public Widget {
public:
  GenericContainer()               noexcept;

  void draw()                      override;
  void noutrefresh()               override;
  bool handle_key(int)             override;
  bool handle_mouse(MEVENT&)       override;
  WINDOW* getWINDOW()              const noexcept override;

  void    add_widget(Widget*);
  Widget* current_widget()         const noexcept;
  void    current_widget(Widget*)        noexcept;
  int     current_index()          const noexcept;
  void    current_index(int)             noexcept;
  int     index_of(Widget*)        const noexcept;
  bool    empty()                  const noexcept;
  int     count()                  const noexcept;
  void    pop_back()                     noexcept;

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
  bool handle_mouse(MEVENT&)       override;
};

} // namespace UI
#endif
