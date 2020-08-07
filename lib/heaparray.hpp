#ifndef LIB_HEAPARRAY_HPP
#define LIB_HEAPARRAY_HPP

/**
 * Heap-allocated array with fixed size
 */
template<typename T>
class HeapArray {
public:
  using value_type = T;
  using size_type = size_t;

  inline HeapArray(size_t size)
  : _data(new T[size]())
  , _size(size)
  {}

  inline ~HeapArray()
  { delete[] _data; }

  // Access operations ========================================================

  inline bool     empty()              const noexcept { return  _size == 0;     }
  inline size_t   size()               const noexcept { return  _size;          }
  inline size_t   max_size()           const noexcept { return  _size;          }
  inline size_t   capacity()           const noexcept { return  _size;          }
  inline T*       begin()                    noexcept { return &_data[0];       }
  inline T const* begin()              const noexcept { return &_data[0];       }
  inline T*       end()                      noexcept { return &_data[_size];   }
  inline T const* end()                const noexcept { return &_data[_size];   }
  inline T&       operator[](size_t i)       noexcept { return  _data[i];       }
  inline T const& operator[](size_t i) const noexcept { return  _data[i];       }
  inline T&       front()                    noexcept { return  _data[0];       }
  inline T const& front()              const noexcept { return  _data[0];       }
  inline T&       back()                     noexcept { return  _data[_size-1]; }
  inline T const& back()               const noexcept { return  _data[_size-1]; }
  inline T*       data()                     noexcept { return &_data[0];       }
  inline T const* data()               const noexcept { return &_data[0];       }

private:
  T*     _data;
  size_t _size;
};

#endif
