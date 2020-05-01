#ifndef LIB_ARRAY_HPP
#define LIB_ARRAY_HPP

/**
 * Wrapper for a dynamically allocated array
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

  size_t   size()                   const noexcept { return _size;         }
  T*       begin()                        noexcept { return &_data[0];     }
  T*       end()                          noexcept { return &_data[_size]; }
  T const* begin()                  const noexcept { return &_data[0];     }
  T const* end()                    const noexcept { return &_data[_size]; }
  T&       operator[](size_t index)       noexcept { return _data[index];  }
  T const& operator[](size_t index) const noexcept { return _data[index];  }

private:
  T*     _data;
  size_t _size;
};

#endif
