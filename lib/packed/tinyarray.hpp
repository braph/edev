#ifndef LIB_PACKED_TINYARRAY_HPP
#define LIB_PACKED_TINYARRAY_HPP

#include "../genericiterator.hpp"
#include "../genericreference.hpp"
#include "../bit_tools.hpp"

#include <initializer_list>

namespace packed {

/* ============================================================================
 * TinyArray - PackedVector's little brother
 * ==========================================================================*/

template<class T>
uint64_t constexpr pack(unsigned i, T* values, unsigned bits, uint64_t result = 0) {
  return i == 0 ? result : pack(i - 1, values, bits, result<<bits | uint64_t(values[i-1]));
}

template<class T, class TStorage, unsigned TBits>
struct TinyArray {
  using value_type       = T;
  using iterator         = GenericIterator<TinyArray>;
  using const_iterator   = GenericConstIterator<TinyArray>;
  using reference        = GenericReference<TinyArray>;
  using const_reference  = GenericConstReference<TinyArray>;
  using size_type        = size_t;

  TStorage _data;
  uint8_t  _size;
  enum { _capacity = sizeof(TStorage) * CHAR_BIT / TBits };

  constexpr TinyArray()           : _data(0), _size(0) {}
  constexpr TinyArray(TStorage v) : _data(v), _size(_capacity) {}
  constexpr TinyArray(const T* v, unsigned N) : _data(pack(N, v, TBits)), _size(N) {}
  constexpr TinyArray(const std::initializer_list<T>& l) : TinyArray(l.begin(), l.end()-l.begin()) {}

  inline void push_back(value_type value) {
    if (_size < _capacity)
      (*this)[_size++] = value;
  }

  inline void set(size_type i, value_type v) { _data = replace_bits<TStorage>(_data, TStorage(v), i * TBits, TBits); }
  inline value_type get(size_type i)         { return value_type(extract_bits<TStorage>(_data, i * TBits, TBits)); }

  inline reference  operator[](size_type i) { return {this, i};  }

  inline size_t     capacity()     const { return _capacity;   }
  inline size_t     size()         const { return _size;       }
  inline TStorage&  data()               { return _data;    }

  inline iterator   begin()              { return {this, 0};         }
  inline iterator   end()                { return {this, _capacity}; }
};

} // namespace packed

#endif
