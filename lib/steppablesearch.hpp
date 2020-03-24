#ifndef LIB_STEPPABLE_SEARCH_HPP
#define LIB_STEPPABLE_SEARCH_HPP

#include "algorithm.hpp" // clamp

#include <functional>

template<typename TContainer>
class SteppableSearch {
  using value_type = typename TContainer::value_type;
  using size_type  = typename TContainer::size_type;
public:
  SteppableSearch() : _list(NULL), _index(0), _predicate(nullptr) {}

  bool next() {
    if (_list && _predicate) {
      _index = clamp<size_type>(_index, 0, _list->size() - 1);

      while (++_index < _list->size())
        if (_predicate((*_list)[_index]))
          return true;

      _index = 0;
    }
    return false;
  }

  bool prev() {
    if (_list && _predicate) {
      _index = clamp<size_type>(_index, 0, _list->size() - 1);

      while (--_index > 0)
        if (_predicate((*_list)[_index]))
          return true;

      _index = _list->size();
    }
    return false;
  }

  void startSearch(const TContainer& list, std::function<bool(const value_type&)> predicate) {
    _index = 0;
    _list = &list;
    _predicate = predicate;
  }

  size_type index() const noexcept
  { return _index; }

  void index(size_type index) noexcept
  { _index = index; }

private:
  const TContainer* _list;
  size_type _index;
  std::function<bool(const value_type&)> _predicate;
};

#endif
