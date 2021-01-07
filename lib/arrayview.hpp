#ifndef ARRAYVIEW_HPP
#define ARRAYVIEW_HPP

#include <cstddef>
#include <iterator>

/**
 * Holds a reference to an array + it's size.
 * The reference as well as the size may be changed at runtime,
 */
template<typename T>
class ArrayView {
public:
#include "inline/contiguous_container_access.hpp"

  inline ArrayView() noexcept
    : _data(NULL)
    , _size(0)
  {}

  inline ArrayView(T* array, size_t N) noexcept
    : _data(array)
    , _size(N)
  {}

  template<size_t N>
  inline ArrayView(T (&array)[N]) noexcept
    : _data(&array[0])
    , _size(N)
  {}

  inline ArrayView& operator=(const ArrayView& rhs) noexcept {
    _data = rhs._data;
    _size = rhs._size;
    return *this;
  }

  template<size_t N>
  inline ArrayView& operator=(T (&array)[N]) noexcept {
    _data = &array[0];
    _size = N;
    return *this;
  }

  inline void shift(size_t n = 1) noexcept { _size -= n; _data += n; }
  inline void pop(size_t n = 1)   noexcept { _size -= n;             }
  inline void size(size_t s)      noexcept { _size = s;              }

private:
  T*     _data;
  size_t _size;
};

#endif
