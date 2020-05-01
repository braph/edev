#ifndef LIB_STATICVECTOR_HPP
#define LIB_STATICVECTOR_HPP

#include <cassert>

/**
 * Stack allocated vector
 * XXX Broken, as it calls default constructor on non existent elements
 */
template<typename T, size_t N>
class StaticVector {
public:
  bool     empty()         const noexcept { return _size == 0;     }
  size_t   size()          const noexcept { return _size;          }
  size_t   max_size()      const noexcept { return N;              }
  size_t   capacity()      const noexcept { return N;              }
  T*       begin()               noexcept { return &_data[0];      }
  T*       end()                 noexcept { return &_data[_size];  }
  T const* begin()         const noexcept { return &_data[0];      }
  T const* end()           const noexcept { return &_data[_size];  }
  T&       operator[](size_t i)  noexcept { return _data[i];       }
  T&       front()               noexcept { return _data[0];       }
  T&       back()                noexcept { return _data[_size-1]; }
  T*       data()                noexcept { return &_data[0];      }

  StaticVector()
  : _size(0)
  {}

  template<typename TIterator>
  StaticVector(TIterator beg, TIterator end) : _size(0) {
    while (beg != end)
      push_back(*beg++);
  }

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

private:
  T      _data[N];
  size_t _size;
};

#endif
