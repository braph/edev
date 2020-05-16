#ifndef UI_MOUSEEVENTS_HPP
#define UI_MOUSEEVENTS_HPP

#include "../ui.hpp"

#include <vector>
#include <algorithm>

namespace UI {

template<typename T>
struct MouseEvents {
  struct Rectangle  {
    UI::Pos start;
    UI::Pos stop;

    Rectangle(const UI::Pos& start, const UI::Pos& stop)
    : start(start)
    , stop(stop)
    {}
  };

  struct MouseEvent {
    Rectangle section;
    T data;

    MouseEvent(const Rectangle& section, T data)
    : section(section)
    , data(std::move(data))
    {}
  };

  using iterator       = typename std::vector<MouseEvent>::iterator;
  using const_iterator = typename std::vector<MouseEvent>::const_iterator;

  void           clear()              noexcept { events.clear();         }
  iterator       begin()              noexcept { return events.begin();  }
  iterator       end()                noexcept { return events.end();    }
  const_iterator begin()        const noexcept { return events.begin();  }
  const_iterator end()          const noexcept { return events.end();    }
  const_iterator cbegin()       const noexcept { return events.cbegin(); }
  const_iterator cend()         const noexcept { return events.cend();   }
  size_t         size()         const noexcept { return events.size();   }
  MouseEvent&    operator[](size_t i) noexcept { return events[i];       }

  iterator find(const Pos& mousePos) noexcept {
    return std::find_if(begin(), end(), [&](const MouseEvent& event) {
        return mousePos >= event.section.start && mousePos <= event.section.stop;
    });
  }

  void add(const Pos& start, const Pos& stop, T data) {
    events.push_back(std::move(MouseEvent(Rectangle(start, stop), std::move(data))));
  }

private:
  std::vector<MouseEvent> events;
};

} // namespace UI

#endif
