#ifndef LIB_STEPPABLE_SEARCH_HPP
#define LIB_STEPPABLE_SEARCH_HPP

#include <functional>
#include <type_traits>

template<typename TContainer>
class SteppableSearch {
  using value_type = typename TContainer::value_type;
  using size_type  = typename TContainer::size_type;

public:
  SteppableSearch() noexcept
    : _list(NULL)
    , _index(0)
    , _predicate(nullptr)
  {}

  void start_search(const TContainer& list, std::function<bool(const value_type&)> predicate) {
    _index = 0;
    _list = &list;
    _predicate = predicate;
  }

  bool next() noexcept {
    if (_list && _predicate) {
      _index = clamp_index(_index);

      while (++_index < _list->size())
        if (_predicate((*_list)[_index]))
          return true;

      _index = 0;
    }
    return false;
  }

  bool prev() noexcept {
    if (_list && _predicate) {
      _index = clamp_index(_index);

      while (_index > 0)
        if (_predicate((*_list)[--_index]))
          return true;

      _index = _list->size() - 1;
    }
    return false;
  }

  size_type index() const noexcept
  { return clamp_index(_index); }

  void index(size_type index) noexcept
  { _index = index; }

private:
  const TContainer* _list;
  size_type _index;
  std::function<bool(const value_type&)> _predicate;

  inline size_type clamp_index(size_type index) const noexcept {
    if (std::is_signed<size_type>::value && index < 0)
      return 0;
    else if (index >= _list->size())
      return _list->size() - 1;
    else
      return index;
  }
};

#endif
