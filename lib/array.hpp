#ifndef LIB_ARRAY_HPP
#define LIB_ARRAY_HPP

/**
 * Wrapper for a dynamically allocated array
 * XXX: WTF This name is utterly bad
 */
template<typename T>
class Array {
public:
  using value_type = T;

  inline Array(size_t size)
  : _data(new T[size]())
  , _size(size)
  {}

 ~Array()
  { delete[] _data; }

  // Access operations ========================================================

  inline bool     empty()              const noexcept { return _size == 0;     }
  inline size_t   size()               const noexcept { return _size;          }
  inline size_t   max_size()           const noexcept { return _size;          } // ?
  inline size_t   capacity()           const noexcept { return _size;          } // ?
  inline T*       begin()                    noexcept { return &_data[0];      }
  inline T const* begin()              const noexcept { return &_data[0];      }
  inline T*       end()                      noexcept { return &_data[_size];  }
  inline T const* end()                const noexcept { return &_data[_size];  }
  inline T&       operator[](size_t i)       noexcept { return _data[i];       }
  inline T const& operator[](size_t i) const noexcept { return _data[i];       }
  inline T&       front()                    noexcept { return _data[0];       }
  inline T const& front()              const noexcept { return _data[0];       }
  inline T&       back()                     noexcept { return _data[_size-1]; }
  inline T const& back()               const noexcept { return _data[_size-1]; }
  inline T*       data()                     noexcept { return &_data[0];      }
  inline T const* data()               const noexcept { return &_data[0];      }

private:
  T*     _data;
  size_t _size;
};

#endif
