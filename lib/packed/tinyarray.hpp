#ifndef LIB_PACKED_TINYARRAY_HPP
#define LIB_PACKED_TINYARRAY_HPP

// TODO!!!!!!

/* ============================================================================
 * TinyArray - PackedVector's little brother
 * ==========================================================================*/

template<class T, class TStorage, unsigned TBits>
struct TinyPackedArray {
  using value_type = T;

  TStorage _data;
  size_t   _size;
  enum { _capacity = sizeof(TStorage) * CHAR_BIT / TBits };

  TinyPackedArray()           : _data(0), _size(0) {}
  TinyPackedArray(TStorage v) : _data(v), _size(_capacity) {}

  inline void push_back(value_type value) {
    if (_size < _capacity)
      (*this)[_size++] = value;
  }

  size_t     capacity()     const { return _capacity;   }
  size_t     size()         const { return _size;       }
  TStorage&  data()               { return _data;    }
  ArrayIterator<TBits, TStorage>   begin()              { return ArrayIterator<TBits, TStorage>(&_data, 0);      }
  ArrayIterator<TBits, TStorage>   end()                { return ArrayIterator<TBits, TStorage>(&_data, size()); }
  ArrayReference<TBits, TStorage>  operator[](size_t i) { return ArrayReference<TBits, TStorage>(&_data, i);     }
};

#endif
