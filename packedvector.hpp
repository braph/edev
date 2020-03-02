#ifndef _PACKED_VECTOR_HPP
#define _PACKED_VECTOR_HPP

#include "common.hpp"
#include "generic.hpp"
#include "bit_tools.hpp"

#include <new>
#include <memory>
#include <iterator>

#ifdef DEBUG_VECTOR
#include <cstdio>
static int call_level = 0;
#define __enter__(FMT, ...) \
  printf("%*s%s(" FMT ")\n", call_level++, "", __PRETTY_FUNCTION__, __VA_ARGS__)
#define __leave__() --call_level
#define debug(FMT, ...) printf("%*s" FMT "\n", call_level, "", __VA_ARGS__) 
#else
#define __enter__(...) (void)0
#define __leave__(...) (void)0
#define debug() (void)0
#endif

/* ============================================================================
 * TinyPackedArray - PackedVector's little brother
 * ==========================================================================*/

template<unsigned TBits, typename TStorage>
struct ArrayReference {
  TStorage* _storage;
  size_t    _i;
  TStorage  _value;

  ArrayReference()
  : _storage(NULL), _i(0), _value(0) {}

  ArrayReference(TStorage* storage, size_t index)
  : _storage(storage), _i(index) {
    _value = extract_bits<TStorage>(*_storage, TBits * _i, TBits);
  }

  ArrayReference(const ArrayReference& rhs)
  : _storage(rhs._storage), _i(rhs._i), _value(rhs._value) {}

  ArrayReference& operator=(TStorage value) {
    *_storage = replace_bits<TStorage>(*_storage, value, TBits * _i, TBits);
    return *this;
  }

  operator TStorage()
  //{ return extract_bits<TStorage>(*_storage, TBits * _i, TBits); }
  { return _value; }
  
  ArrayReference& operator=(const ArrayReference& rhs)
  { return *this = rhs._value; }

#if 0
  ArrayReference& operator=(const ArrayReference&& rhs)
  { return *this = rhs.get(); }
#endif

#if 0
  inline TStorage get() const
  { return extract_bits<TStorage>(*_storage, TBits * _i, TBits); }
#endif

  void load(TStorage* _storage, size_t _i) {
    this->_storage = _storage;
    this->_i = _i;
    this->_value = extract_bits<TStorage>(*_storage, TBits * _i, TBits);
  }
};

template<unsigned TBits, typename TStorage>
struct ArrayIterator { //: public ArrayReference<TBits, TStorage> {
  TStorage* _storage;
  size_t    _i;

  ArrayReference<TBits, TStorage> ref;

  using iterator_category = std::random_access_iterator_tag;
  using value_type        = ArrayReference<TBits, TStorage>;
  using difference_type   = std::ptrdiff_t;
  using pointer           = ArrayReference<TBits, TStorage>;//*;
  using reference         = ArrayReference<TBits, TStorage>&;//&;

  ArrayIterator(TStorage* storage, size_t index)
  : _storage(storage), _i(index) {}

  ArrayIterator(const ArrayIterator<TBits, TStorage>& rhs)
  : _storage(rhs._storage), _i(rhs._i) {}

  ArrayIterator& operator=(const ArrayIterator&I)
  { _i = I._i; return *this; }

  inline bool operator==(const ArrayIterator&I)  { return _i == I._i; }
  inline bool operator!=(const ArrayIterator&I)  { return _i != I._i; }
  inline bool operator<=(const ArrayIterator&I)  { return _i <= I._i; }
  inline bool operator>=(const ArrayIterator&I)  { return _i >= I._i; }
  inline bool operator< (const ArrayIterator&I)  { return _i <  I._i; }
  inline bool operator> (const ArrayIterator&I)  { return _i >  I._i; }

  inline ArrayIterator& operator++()             { ++_i; return *this; }
  inline ArrayIterator& operator--()             { --_i; return *this; }
  inline ArrayIterator  operator++(int)    { ArrayIterator I = *this; ++_i; return I; }
  inline ArrayIterator  operator--(int)    { ArrayIterator I = *this; --_i; return I; }
  inline ArrayIterator& operator+=(ptrdiff_t n)       { _i += size_t(n); return *this; }
  inline ArrayIterator& operator-=(ptrdiff_t n)       { _i -= size_t(n); return *this; }
  inline ArrayIterator  operator+ (ptrdiff_t n) const { ArrayIterator I = *this; return I += n; }
  inline ArrayIterator  operator- (ptrdiff_t n) const { ArrayIterator I = *this; return I -= n; }

  inline ArrayReference<TBits, TStorage>& operator*() {
    ref.load(_storage, _i);
    return ref;
  }
  inline ArrayReference<TBits, TStorage>& operator[](ptrdiff_t n)       { return *(*this + n);               }

  inline ptrdiff_t operator- (const ArrayIterator&I) const
  { return ptrdiff_t(_i) - ptrdiff_t(I._i); }

  inline ptrdiff_t operator+ (const ArrayIterator&I) const
  { return ptrdiff_t(_i) + ptrdiff_t(I._i); }
};

template<unsigned TBits, typename TStorage>
struct TinyPackedArray {
  TStorage     _storage;
  size_t       _size;
  enum { _capacity = elements_fit_in_bits(TBits, sizeof(TStorage)*8) };

  TinyPackedArray()           : _storage(0), _size(0) {}
  TinyPackedArray(TStorage v) : _storage(v), _size(_capacity) {}

  inline void push_back(TStorage value) {
    if (_size < _capacity)
      (*this)[_size++] = value;
  }

  using value_type        = ArrayReference<TBits, TStorage>;

  inline size_t     capacity()     const { return _capacity;   }
  inline size_t     size()         const { return _size;       }
  inline void       fullSize()           { _size = _capacity;  }
  inline TStorage&  data()               { return _storage;    }
  ArrayIterator<TBits, TStorage>   begin()              { return ArrayIterator<TBits, TStorage>(&_storage, 0);      }
  ArrayIterator<TBits, TStorage>   end()                { return ArrayIterator<TBits, TStorage>(&_storage, size()); }
  ArrayReference<TBits, TStorage>  operator[](size_t i) { return ArrayReference<TBits, TStorage>(&_storage, i);     }
};

#include<iostream>
#if 0
namespace std{
#if 0
  template<typename TBits, typename TStorage>
  void iter_swap(ArrayIterator<TBits, TStorage> it1, ArrayIterator<TBits, TStorage> it2) {
  }
#endif

  template<typename TBits, typename TStorage>
  void swap(ArrayReference<TBits, TStorage> &a, ArrayReference<TBits, TStorage>& b) {
    std::cout << "sdfsf" << std::endl;
  }
}
#endif

/* ============================================================================
 * PackedVector
 * ==========================================================================*/

class PackedVector {
  using storage_type = uint32_t;

  storage_type* _data;
  size_t   _size;     // element count
  size_t   _capacity; // element count
  uint8_t  _bits;

public:
  using iterator   = GenericIterator<PackedVector>;
  using reference  = GenericReference<PackedVector>;
  using value_type = int;

  PackedVector(int bits)
  : _data(NULL)
  , _size(0)
  , _capacity(0)
  , _bits(unsigned(bits)) {
    __enter__("bits = %u", bits);
    if (bits > 31 || bits < 1)
      throw std::invalid_argument("bits > 31 || bits < 1");
    __leave__();
  }

  PackedVector(PackedVector&& rhs)
  : _data(rhs._data)
  , _size(rhs._size)
  , _capacity(rhs._capacity)
  , _bits(rhs._bits) {
    __enter__("%s", "rhs");
    rhs._data = NULL;
    rhs._size = 0;
    rhs._capacity = 0;
    __leave__();
  }

 ~PackedVector() {
    if (_data)
      delete[] _data;
  }

  PackedVector& operator=(PackedVector&& rhs) {
    __enter__("%s", "rhs");
    std::swap(_data, rhs._data);
    std::swap(_size, rhs._size);
    std::swap(_capacity, rhs._capacity);
    _bits = rhs._bits;
    __leave__();
    return *this;
  }

  reference operator[](size_t idx) { return reference(*this, idx);    }
  iterator  begin()                { return iterator(*this, 0);       }
  iterator  end()                  { return iterator(*this, size());  }

  size_t        size()      const  { return _size;      }
  bool          empty()     const  { return _size == 0; }
  void          clear()            { _size = 0;         } 
  storage_type* data()             { return _data;      }
  size_t        capacity()  const  { return _capacity;  }
  void          shrink_to_fit()    { /* TODO */         }
  
  void    push_back(value_type);
  void    reserve(size_t);
  void    resize(size_t n, value_type value = 0);

  // PackedVector specific methods (not available in std::vector)
  int         bits()      const   { return _bits; }
  value_type  get(size_t) const;
  void        set(size_t, value_type);

  friend class DynamicPackedVector;

protected:
  // Private copy constructor
  PackedVector(int bits, size_t capacity, iterator begIt, iterator endIt)
  : _data(new storage_type[size_for_bits(unsigned(bits)*capacity, sizeof(storage_type))])
  , _size(0)
  , _capacity(capacity)
  , _bits(bits)
  {
    __enter__("(priv) %d, %lu, iterator, iterator", bits, capacity);

    while (begIt != endIt)
      push_back(*begIt++);

    __leave__();
  }
};

/* ============================================================================
 * DynamicPackedVector
 * ==========================================================================*/

class DynamicPackedVector {
private:
  PackedVector _vec;
public:
  DynamicPackedVector() : _vec(1) { }

  using storage_type = uint32_t;
  using reference    = GenericReference<DynamicPackedVector>;
  using iterator     = GenericIterator<DynamicPackedVector>;
  using value_type   = int;

  reference operator[](size_t index) { return reference(*this, index); }
  iterator  begin()                  { return iterator(*this, 0);      }
  iterator  end()                    { return iterator(*this, size()); }

  // Proxy methods to PackedVector ============================================
  void            clear()                  { _vec.clear();             }
  size_t          size()           const   { return _vec.size();       }
  bool            empty()          const   { return _vec.empty();      }
  size_t          capacity()       const   { return _vec.capacity();   }
  storage_type*   data()                   { return _vec.data();       }
  int             bits()           const   { return _vec.bits();       }
  value_type      get(size_t idx)  const   { return _vec.get(idx);     }

  // Methods that may replace the underlying _vec object
  void reserve(size_t, int bits = 1);
  void resize(size_t, value_type value = 0);
  void set(size_t, value_type);
  void push_back(value_type);
  void shrink_to_fit();
};

#endif

