#ifndef LIB_STATICVECTOR_HPP
#define LIB_STATICVECTOR_HPP

#include <cassert>

/* Statically allocated vector */
template<typename T, size_t N>
class StaticVector {
  T      _data[N];
  size_t _size;
public:
  StaticVector() : _size(0) {}
  bool     empty()         const noexcept { return _size == 0;      }
  size_t   size()          const noexcept { return _size;           }
  size_t   max_size()      const noexcept { return N;               }
  size_t   capacity()      const noexcept { return N;               }
  T*       begin()               noexcept { return &_data[0];       }
  T*       end()                 noexcept { return &_data[size()];  }
  T&       operator[](size_t i)  noexcept { return _data[i];        }
  T&       front()               noexcept { return _data[0];        }
  T&       back()                noexcept { return _data[size()-1]; }
  T*       data()                noexcept { return &_data[0];       }

  template<typename TIterator>
  StaticVector(TIterator beg, TIterator end) : _size(0) {
    while (beg != end)
      push_back(*beg++);
  }

  void push_back(const T& e) {
    assert(size() < max_size());
    _data[_size++] = e;
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
};

#endif
