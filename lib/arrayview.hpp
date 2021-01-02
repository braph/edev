#ifndef ARRAYVIEW_HPP
#define ARRAYVIEW_HPP

/**
 * Holds a reference to an array.
 * - Provides size()
 * - Maybe add begin() and end()?
 */
template<typename T>
class ArrayView {
public:
  using value_type = T;

  ArrayView() noexcept
    : _data(NULL)
    , _size(0)
  {}

  ArrayView(T* array, size_t N)
    : _data(array)
    , _size(N)
  {}

  template<size_t N>
  ArrayView(T (&array)[N]) noexcept
    : _data(&array[0])
    , _size(N)
  {}

  value_type& operator[](size_t index) noexcept
  { return _data[index]; }

  const value_type& operator[](size_t index) const noexcept
  { return _data[index]; }

  size_t size() const noexcept
  { return _size; }

  void size(size_t s) noexcept
  { _size = s; }

  inline void shift(size_t n = 1) noexcept { _size -= n; _data += n; }
  inline void pop(size_t n = 1)   noexcept { _size -= n;             }

#ifndef TODO
  inline T*       begin()                    noexcept { return &_data[0];      }
  inline T const* begin()              const noexcept { return &_data[0];      }
  inline T*       end()                      noexcept { return &_data[_size];  }
  inline T const* end()                const noexcept { return &_data[_size];  }
  inline T&       front()                    noexcept { return _data[0];       }
  inline T const& front()              const noexcept { return _data[0];       }
  inline T&       back()                     noexcept { return _data[_size-1]; }
  inline T const& back()               const noexcept { return _data[_size-1]; }
  inline T*       data()                     noexcept { return &_data[0];      }
  inline T const* data()               const noexcept { return &_data[0];      }
#endif
private:
  T*     _data;
  size_t _size;
};

#endif
