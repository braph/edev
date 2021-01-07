#ifndef LIB_HEAPARRAY_HPP
#define LIB_HEAPARRAY_HPP

#include <cstddef>
#include <iterator>

/**
 * Heap-allocated array with fixed size.
 * XXX: Allocated array will get default-initialized!
 */
template<typename T>
class HeapArray {
public:
#include "inline/contiguous_container_access.hpp"

  inline HeapArray(size_t size)
  : _data(new T[size]())
  , _size(size)
  {}

  inline ~HeapArray()
  { delete[] _data; }

private:
  T*     _data;
  size_t _size;
};

#endif
