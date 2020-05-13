#ifndef LIB_ITERATOR_PAIR_HPP
#define LIB_ITERATOR_PAIR_HPP

/**
 * Pack two iterators into one object
 */

template<typename TIterator>
struct IteratorPair {
  TIterator _it;
  TIterator _end;

  // Constructors =============================================================
  template<typename TContainer>
  inline IteratorPair(TContainer& container)
    : _it(container.begin())
    , _end(container.end())
  {}

  inline IteratorPair(TIterator begin, TIterator end) noexcept
    : _it(begin)
    , _end(end)
  {}

  // Has next =================================================================
  inline bool has_next() const noexcept
  { return _it != _end; }

  inline bool operator!() const noexcept
  { return _it == _end; }

  inline operator bool() const noexcept
  { return _it != _end; }

  // Dereferencing ============================================================
  inline decltype(*_it) operator*() const noexcept {
    return *_it;
  }

  inline decltype(*_it) next() noexcept {
    decltype(*_it) value(*_it);
    ++_it;
    return value;
  }

  // Range for ================================================================
  inline TIterator begin() const noexcept
  { return _it; }

  inline TIterator end() const noexcept
  { return _end; }
};

template<typename TContainer>
inline IteratorPair<typename TContainer::iterator>
make_iterator_pair(TContainer& container) {
  return IteratorPair<typename TContainer::iterator>(container);
}

template<typename TIterator>
inline IteratorPair<TIterator>
make_iterator_pair(TIterator begin, TIterator end) {
  return IteratorPair<TIterator>(begin, end);
}

#endif

#ifdef TEST_LIB_ITERATOR_PAIR
#include <vector>
#include <iostream>

int main() {
  std::vector<int> v = {1,2,3,4};

  IteratorPair<std::vector<int>::iterator> pair(v.begin(), v.end());
  IteratorPair<std::vector<int>::iterator> pair2(v);

  while (pair) {
    std::cout << pair.next() << std::endl;
  }
}
#endif
