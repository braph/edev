#ifndef UI_MOUSEEVENTS_HPP
#define UI_MOUSEEVENTS_HPP

#include "../ui.hpp"

#include <vector>
#include <algorithm>

namespace UI {

template<typename T>
struct MouseEvents {
  struct Rectangle  {
    UI::Pos start, stop;
    Rectangle(const UI::Pos& start, const UI::Pos& stop) : start(start), stop(stop) {}
  };

  struct MouseEvent {
    Rectangle section;
    T data;
    MouseEvent(const Rectangle& section, const T& data) : section(section), data(data) {}
  };

  std::vector<MouseEvent> events;

  using iterator = typename std::vector<MouseEvent>::iterator;

  size_t   size()   noexcept { return events.size();  }
  iterator begin()  noexcept { return events.begin(); }
  iterator end()    noexcept { return events.end();   }
  iterator cbegin() noexcept { return events.begin(); }
  iterator cend()   noexcept { return events.end();   }
  void     clear()  noexcept { events.clear();        }

  iterator find(const Pos& mousePos) noexcept {
    return std::find_if(begin(), end(), [&](const MouseEvent& event) {
        return mousePos >= event.section.start && mousePos <= event.section.stop; });
  }

  void add(const Pos& start, const Pos& stop, const T& data) {
    events.push_back(MouseEvent(Rectangle(start, stop), data));
  }
};

} // namespace UI

#endif
