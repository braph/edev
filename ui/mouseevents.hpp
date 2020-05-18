#ifndef UI_MOUSEEVENTS_HPP
#define UI_MOUSEEVENTS_HPP

#include "../ui.hpp"

#include <vector>
#include <algorithm>

namespace UI {

template<typename T>
struct MouseEvents {
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

  void           clear()              noexcept { _events.clear();         }
  iterator       begin()              noexcept { return _events.begin();  }
  iterator       end()                noexcept { return _events.end();    }
  const_iterator begin()        const noexcept { return _events.begin();  }
  const_iterator end()          const noexcept { return _events.end();    }
  const_iterator cbegin()       const noexcept { return _events.cbegin(); }
  const_iterator cend()         const noexcept { return _events.cend();   }
  size_t         size()         const noexcept { return _events.size();   }
  MouseEvent&    operator[](size_t i) noexcept { return _events[i];       }

  iterator find(const Pos& mousePos) noexcept {
    return std::find_if(begin(), end(), [&](const MouseEvent& event) {
      return event.section.encloses(mousePos);
    });
  }

  void add(const Pos& start, const Pos& stop, T data) {
    _events.push_back(std::move(MouseEvent(Rectangle(start, stop), std::move(data))));
  }

private:
  std::vector<MouseEvent> _events;
};

} // namespace UI

#endif
