#ifndef LIB_STATICVECTOR_HPP
#define LIB_STATICVECTOR_HPP

#include <array>
#include <cassert>

/**
 * Stack allocated vector
 * XXX Broken, as it calls default constructor on non existent elements
 * XXX This is generally utterly broken.
 */
template<typename T, size_t N>
class StaticVector {
public:
  using value_type = T;

  // Constructors =============================================================

  StaticVector() noexcept
  : _size(0)
  {}

  template<typename TIterator>
  StaticVector(TIterator beg, TIterator end) : _size(0) {
    while (beg != end)
      push_back(*beg++);
  }

  StaticVector(const std::initializer_list<T> &list)
    : StaticVector(list.begin(), list.end())
  {}

  StaticVector(const StaticVector& rhs)
    : _data(rhs._data)
    , _size(rhs._size)
  {}

  // Assignment operator ======================================================

  StaticVector& operator=(const StaticVector& rhs) {
    _data = rhs._data;
    _size = rhs._size;
    return *this;
  }

  StaticVector& operator=(StaticVector&& rhs) {
    _data = std::move(rhs._data);
    _size = rhs._size;
    return *this;
  }

  // Modify operations ========================================================

  void push_back(const T& e) {
    assert(size() < max_size());
    _data[_size++] = e;
  }

  void push_back(T&& e) {
    assert(size() < max_size());
    _data[_size++] = std::move(e);
  }

  void resize(size_t n) {
    assert(n <= max_size());
    _size = n;
  }

  void resize(size_t n, const T& e) {
    assert(n <= max_size());
    while (_size < n)
      push_back(e);
  }

  void reserve(size_t n) {
    return; // TODO
  }

  // Access operations ========================================================

  inline bool     empty()              const noexcept { return _size == 0;     }
  inline size_t   size()               const noexcept { return _size;          }
  inline size_t   max_size()           const noexcept { return N;              }
  inline size_t   capacity()           const noexcept { return N;              }
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
  std::array<T, N> _data;
  size_t _size;
};

#endif
