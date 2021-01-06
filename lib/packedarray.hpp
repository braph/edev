#ifndef LIB_PACKEDARRAY_HPP
#define LIB_PACKEDARRAY_HPP

// TODO!!!!!!

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

  bool operator==(const ArrayIterator&I)  { return _i == I._i; }
  bool operator!=(const ArrayIterator&I)  { return _i != I._i; }
  bool operator<=(const ArrayIterator&I)  { return _i <= I._i; }
  bool operator>=(const ArrayIterator&I)  { return _i >= I._i; }
  bool operator< (const ArrayIterator&I)  { return _i <  I._i; }
  bool operator> (const ArrayIterator&I)  { return _i >  I._i; }

  ArrayIterator& operator++()             { ++_i; return *this; }
  ArrayIterator& operator--()             { --_i; return *this; }
  ArrayIterator  operator++(int)    { ArrayIterator I = *this; ++_i; return I; }
  ArrayIterator  operator--(int)    { ArrayIterator I = *this; --_i; return I; }
  ArrayIterator& operator+=(ptrdiff_t n)       { _i += size_t(n); return *this; }
  ArrayIterator& operator-=(ptrdiff_t n)       { _i -= size_t(n); return *this; }
  ArrayIterator  operator+ (ptrdiff_t n) const { ArrayIterator I = *this; return I += n; }
  ArrayIterator  operator- (ptrdiff_t n) const { ArrayIterator I = *this; return I -= n; }

  ArrayReference<TBits, TStorage>& operator*() {
    ref.load(_storage, _i);
    return ref;
  }
  ArrayReference<TBits, TStorage>& operator[](ptrdiff_t n)
  { return *(*this + n); }

  ptrdiff_t operator- (const ArrayIterator&I) const noexcept
  { return ptrdiff_t(_i) - ptrdiff_t(I._i); }

  ptrdiff_t operator+ (const ArrayIterator&I) const noexcept
  { return ptrdiff_t(_i) + ptrdiff_t(I._i); }
};

template<unsigned TBits, typename TStorage>
struct TinyPackedArray {
  TStorage     _storage;
  size_t       _size;
  enum { _capacity = sizeof(TStorage)*CHAR_BIT / TBits };

  TinyPackedArray()           : _storage(0), _size(0) {}
  TinyPackedArray(TStorage v) : _storage(v), _size(_capacity) {}

  inline void push_back(TStorage value) {
    if (_size < _capacity)
      (*this)[_size++] = value;
  }

  using value_type        = ArrayReference<TBits, TStorage>;

  size_t     capacity()     const { return _capacity;   }
  size_t     size()         const { return _size;       }
  void       fullSize()           { _size = _capacity;  }
  TStorage&  data()               { return _storage;    }
  ArrayIterator<TBits, TStorage>   begin()              { return ArrayIterator<TBits, TStorage>(&_storage, 0);      }
  ArrayIterator<TBits, TStorage>   end()                { return ArrayIterator<TBits, TStorage>(&_storage, size()); }
  ArrayReference<TBits, TStorage>  operator[](size_t i) { return ArrayReference<TBits, TStorage>(&_storage, i);     }
};

#endif
