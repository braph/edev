#ifndef LIB_ARRAY_HPP
#define LIB_ARRAY_HPP

/**
 * Wrapper for a dynamically allocated array
 * XXX: This name is utterly bad
 */
template<typename T>
class Array {
public:
  using value_type = T;

  Array(size_t size)
  : _data(new T[size]())
  , _size(size)
  {}

 ~Array()
  { delete[] _data; }

  // Access operations ========================================================

  bool     empty()              const noexcept { return _size == 0;     }
  size_t   size()               const noexcept { return _size;          }
  size_t   max_size()           const noexcept { return _size;          } // ?
  size_t   capacity()           const noexcept { return _size;          } // ?
  T*       begin()                    noexcept { return &_data[0];      }
  T const* begin()              const noexcept { return &_data[0];      }
  T*       end()                      noexcept { return &_data[_size];  }
  T const* end()                const noexcept { return &_data[_size];  }
  T&       operator[](size_t i)       noexcept { return _data[i];       }
  T const& operator[](size_t i) const noexcept { return _data[i];       }
  T&       front()                    noexcept { return _data[0];       }
  T const& front()              const noexcept { return _data[0];       }
  T&       back()                     noexcept { return _data[_size-1]; }
  T const& back()               const noexcept { return _data[_size-1]; }
  T*       data()                     noexcept { return &_data[0];      }
  T const* data()               const noexcept { return &_data[0];      }

private:
  T*     _data;
  size_t _size;
};

#endif
