#ifndef LIB_STATICVECTOR_HPP
#define LIB_STATICVECTOR_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>

/**
 * Stack allocated vector
 * XXX: This vector is only usable for POD types...
 * XXX: Constant initialization(?) like in std::array not possible
 */
template<typename T, size_t N>
class StaticVector {
public:
#include "inline/contiguous_container_access.hpp"

  inline StaticVector() noexcept
  : _size(0)
  {}

  template<typename TIterator>
  inline StaticVector(TIterator beg, TIterator end) : _size(0) {
    while (beg != end)
      push_back(*beg++);
  }

  inline StaticVector(const std::initializer_list<T> &list)
    : StaticVector(list.begin(), list.end())
  {}

  inline StaticVector(const StaticVector& rhs)
    : _data(rhs._data)
    , _size(rhs._size)
  {}

  // Assignment operator ======================================================

  inline StaticVector& operator=(const StaticVector& rhs) {
    _data = rhs._data;
    _size = rhs._size;
    return *this;
  }

  inline StaticVector& operator=(StaticVector&& rhs) {
    _data = std::move(rhs._data);
    _size = rhs._size;
    return *this;
  }

  // Modify operations ========================================================

  inline void push_back(const T& e) {
    assert(size() < max_size());
    _data[_size++] = e;
  }

  inline void push_back(T&& e) {
    assert(size() < max_size());
    _data[_size++] = std::move(e);
  }

  inline void resize(size_t n) {
    assert(n <= max_size());
    _size = n;
  }

  inline void resize(size_t n, const T& e) {
    assert(n <= max_size());
    while (_size < n)
      push_back(e);
  }

  inline void reserve(size_t n) {
    assert(n <= size());
  }

  inline size_t max_size()  const noexcept { return N; }
  inline size_t capacity()  const noexcept { return N; }

private:
  std::array<T, N> _data;
  size_t           _size;
};

#endif
