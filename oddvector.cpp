#include <vector>
#include <cinttypes>
#include <iostream>
#include <stdexcept>
#include "database.hpp" // GenericIterator

// XXX: Only 31-bit values supported.
// XXX: Overflow on large keysets (N * 32 / bits)

static inline uint32_t bitlength_32(uint32_t n) {
  return (n ? 32 - __builtin_clz(n) : 0);
}

static inline uint32_t replace_bits_32(uint32_t src, uint32_t val, uint32_t offset, uint32_t len) {
  if (len == 32)
    return val;
  uint32_t mask = (~(0xFFFFFFFFu << len)) << offset;
  return (src & ~mask) | (val << offset);
}

static inline uint64_t replace_bits_64(uint64_t src, uint64_t val, uint64_t offset, uint64_t len) {
  if (len == 64)
    return val;
  uint64_t mask = (~(0xFFFFFFFFFFFFFFFFu << len)) << offset;
  return (src & ~mask) | (val << offset);
}

union BitShiftHelper {
  struct {
    uint32_t u1;
    uint32_t u2;
  } u32;
  uint64_t u64;
};

class PackedVector {
public:
  uint8_t   _bits;
  size_t    _size;
  std::vector<uint32_t> _data;

public:
  PackedVector(int bits) : _bits(bits), _size(0) {
    if (bits > 31 || bits < 1)
      throw std::invalid_argument("31 > bits > 0");
  }

  bool    empty()     const    { return _size == 0;                     }
  size_t  size()      const    { return _size;                          }
  size_t  capacity()  const    { return _data.capacity() * 32 / _bits;  }
  void    clear()              { _size = 0;                             } 

  void reserve(size_t n) {
    if (n)
      _data.reserve((n+1) * _bits / 32 - 1);
  }

  void resize(size_t n) {
    if (n) // Vector may grow 1 byte larger than actually needed
      _data.resize(n * _bits / 32 + 1);
    _size = n;
  }

  void      set(size_t index, int value);
  uint32_t  get(size_t index) const;
  void      push_back(const int &value);

  //reference at(size_t pos);

  class reference { // _GLIBCXX_NOEXCEPT on reference()
    friend class PackedVector;
    PackedVector&  _oddVector;
    size_t      _index;
    reference();
  public:
    reference(PackedVector &vec, size_t idx) : _oddVector(vec), _index(idx) {}
#if __cplusplus >= 201103L
    reference(const reference&) = default;
#endif
    ~reference() _GLIBCXX_NOEXCEPT
    { }

    reference& operator=(int value) _GLIBCXX_NOEXCEPT {
      _oddVector.set(_index, value);
      return *this;
    }

    reference& operator=(const reference& rhs) _GLIBCXX_NOEXCEPT {
      _oddVector.set(_index, rhs._oddVector[rhs._index]);
      return *this;
    }

    operator int() const _GLIBCXX_NOEXCEPT {
      return _oddVector.get(_index);
    }
  };

  class const_reference {
    friend class PackedVector;
    const PackedVector &_oddVector;
    size_t _index;
    const_reference();
  public:
    const_reference(const PackedVector &vec, size_t idx) : _oddVector(vec), _index(idx) {}
#if __cplusplus >= 201103L
    const_reference(const const_reference&) = default;
#endif
    ~const_reference() _GLIBCXX_NOEXCEPT
    { }

#if 0
    reference& operator=(int value) _GLIBCXX_NOEXCEPT {
      _oddVector.set(_index, value);
      return *this;
    }

    reference& operator=(const reference& rhs) _GLIBCXX_NOEXCEPT {
      _oddVector.set(_index, rhs._oddVector[rhs._index]);
      return *this;
    }
#endif

    operator int() const _GLIBCXX_NOEXCEPT {
      return _oddVector.get(_index);
    }
  };

  friend class reference;
  friend class const_reference;

  reference operator[](int index) { return reference(*this, index); }
  const_reference operator[](int index) const { return const_reference(*this, index); }
};

uint32_t PackedVector :: get(size_t index) const {
  uint32_t dataIndex   = (index * _bits) / 32;
  uint32_t bitOffset   = (index * _bits) % 32;

  if (bitOffset + _bits <= 32) {
    uint32_t e = _data[dataIndex];
    return (e >> bitOffset) &~(0xFFFFFFFFu << _bits);
  } else {
    BitShiftHelper store;
    store.u32.u1 = _data[dataIndex];
    store.u32.u2 = _data[dataIndex+1];
    return (store.u64 >> bitOffset) &~(0xFFFFFFFFu << _bits);
  }
}

void PackedVector :: set(size_t index, int value) {
  uint32_t dataIndex   = (index * _bits) / 32;
  uint32_t bitOffset   = (index * _bits) % 32;

  /*
  std::cout
    << "set(index=" << index << ", value="  << value
    << " dataIndex=" << dataIndex
    << " bitOffset="   << bitOffset
    << ")\n"; */

  if (bitOffset + _bits <= 32) {
    uint32_t e = _data[dataIndex];
    _data[dataIndex] = replace_bits_32(e, value, bitOffset, _bits);
  } else {
    BitShiftHelper store;
    store.u32.u1 = _data[dataIndex];
    store.u32.u2 = _data[dataIndex+1];
    store.u64 = replace_bits_64(store.u64, value, bitOffset, _bits);
    _data[dataIndex] = store.u32.u1;
    _data[dataIndex+1] = store.u32.u2;
  }
}

void PackedVector :: push_back(const int &value) {
  resize(_size + 1);
  set(_size - 1, value);
}

#if TEST_ODDVECTOR
#include <cassert>
#include <iostream>
#include <iomanip>

void dump(PackedVector &odd) {
  std::cout << "DUMP" << std::endl;
  for (uint32_t i : odd._data) {
    std::cout << '[' << std::setw(5) << i << ']' << ' ';
    for (int n = 32; n--;) {
      std::cout << ((i >> n) & 1U);
      if (n % 8 == 0)
        std::cout << ' ';
    }
    std::cout << '|';
  }
  std::cout << std::endl;
}

bool strVecEqual(const std::vector<uint32_t> &a, const PackedVector &b) {
  if (a.size() != b.size())
    return false;
  for (auto i = a.size(); i--;)
    if (a[i] != b[i])
      return false;
  return true;
}

#define CHECK check(vec, odd)
static inline void check(std::vector<uint32_t> &vec, PackedVector &odd) {
  assert(vec.size()     ==  odd.size());
  assert(vec.empty()    ==  odd.empty());
  //assert(vec.capacity() ==  odd.capacity()); XXX capacity may change
}

#define TEST(WHAT)      vec.WHAT; odd.WHAT; CHECK
#define TEST_INT(WHAT)  assert((int) vec.WHAT == (int) odd.WHAT); CHECK

int main() {
  // Test if the implementation behaves like a normal vector for bit sizes
  // 32 to 1.


  std::vector<int> v10;
  for (int i = 0; i < 10; ++i) {
    v10.push_back(i);
  }

  PackedVector foo(20);

  std::copy(v10.begin(), v10.end(), std::back_inserter(foo));



  PackedVector t(14);
  t.push_back(2);
  std::cout << t[0] << std::endl;
  dump(t);
  t.push_back(3);
  std::cout << t[1] << std::endl;
  dump(t);
  t.push_back(3);
  std::cout << t[2] << std::endl;
  dump(t);

  t[0] = 5;
  std::cout << t[0] << std::endl;

  for (int bits = 31; bits; --bits) {
    std::cout << "Testing with " << bits << " bits." << std::endl;

    std::vector<uint32_t> vec;
    PackedVector             odd(bits);
    CHECK;

    //vec.reserve(0);
    //TEST(reserve(0));

    //TEST(reserve(1));

    //TEST(reserve(10));
    //TEST(resize(0));

    //TEST(reserve(1));
    //TEST(resize(1));

    TEST(push_back(1));
    dump(odd);
    std::cout << odd[0] << ':' << vec[0] << std::endl;
    TEST_INT(operator[](0));

    TEST(push_back(2));
    dump(odd);
    std::cout << vec[0] << ':' << odd[0] << std::endl;
    std::cout << vec[1] << ':' << odd[1] << std::endl;
    TEST_INT(operator[](0));
    TEST_INT(operator[](1));
  }

#if 0
  /*********/ vec.reserve(1);
  assert(0 == vec.size());
  assert(1 == vec.capacity());
  /*********/ vec.resize(1);
  assert(0 == vec[0]);

  // === Test the PackedVector ===
  assert(0 == odd.size());
  assert(0 == odd.v.size());
  assert(0 == odd.capacity());
  assert(0 == odd.v.capacity());
  /*********/ odd.reserve(1);
  assert(0 == odd.size());
  assert(0 == odd.v.size());
  assert(1 == odd.capacity());
  assert(1 == odd.v.capacity());
  /*********/ odd.resize(1);
  assert(1 == odd.size());
  assert(1 == odd.v.size());
  assert(1 == odd.capacity());
  assert(1 == odd.v.capacity());
  assert(0 == odd[0]);
#endif

  //PackedVector t0(2);
  //assert(t0.size() == 0);
}
#endif




#if FUCK
  if (bitOffset + bits <= 32) {

    uint32_t e = v[dataIndex];
    uint32_t clearMask =
      FILLBITS_LEFT(bitOffset)|FILLBITS_RIGHT(32-bits-bitOffset);
    e &= clearMask;
    e |= (value << 32-bits-bitOffset);


    v[dataIndex] =
      // Preserve left bits
      ((e >> lShift) << lShift)     |
      // Foo
      (value << (32 - bitOffset))   |
      // Bar
      ((e & (
      v[dataIndex] 

// I'm an idiot. This stuff can for sure be optimized.
#define FULLBITS          ((uint32_t) 0xFFFFFFFF) //        1111
#define FILLBITS_LEFT(N)  (FULLBITS << (32 - N))  // N=1 -> 1000
#define FILLBITS_RIGHT(N) (FULLBITS >> (32 - N))  // N=1 -> 0001

  uint32_t bitOffset   = (index * bits) % 32;
  uint32_t dataIndex = (index * bits - bitOffset) / 32;

  if (bitOffset + bits <= 32) {
    uint32_t elm    = v[dataIndex];
    uint32_t lShift = bitOffset;
    uint32_t rShift = 32 - (bitOffset + bits);
    return ((elm << lShift) >> lShift >> rShift);
  } else {
    uint32_t overlap = bitOffset + bits - 32;
    uint32_t left    = v[dataIndex]   << bitOffset;
    uint32_t right   = v[dataIndex+1] >> (32 - overlap);
    return (left >> (32 - bits)) | right;
  }
#endif





#if 0
  typedef _Storage_type uint32_t;

  struct _Packed_iterator_base
    : public std::iterator<std::random_access_iterator_tag, uint32_t>
  {
    _Storage_type * _M_p;
    unsigned int _M_offset;
    unsigned int _M_bitlength;

    _Packed_iterator_base(_Storage_type * __x, unsigned int __y)
      : _M_p(__x), _M_offset(__y) { }

    void
      _M_bump_up()
      {
        if (_M_offset++ == int(_S_word_bit) - 1)
        {
          _M_offset = 0;
          ++_M_p;
        }
      }

    void
      _M_bump_down()
      {
        if (_M_offset-- == 0)
        {
          _M_offset = int(_S_word_bit) - 1;
          --_M_p;
        }
      }

    void
      _M_incr(ptrdiff_t __i)
      {
        difference_type __n = __i + _M_offset;
        _M_p += __n / int(_S_word_bit);
        __n = __n % int(_S_word_bit);
        if (__n < 0)
        {
          __n += int(_S_word_bit);
          --_M_p;
        }
        _M_offset = static_cast<unsigned int>(__n);
      }

    bool
      operator==(const _Packed_iterator_base& __i) const
      { return _M_p == __i._M_p && _M_offset == __i._M_offset; }

    bool
      operator<(const _Packed_iterator_base& __i) const
      {
        return _M_p < __i._M_p
          || (_M_p == __i._M_p && _M_offset < __i._M_offset);
      }

    bool
      operator!=(const _Packed_iterator_base& __i) const
      { return !(*this == __i); }

    bool
      operator>(const _Packed_iterator_base& __i) const
      { return __i < *this; }

    bool
      operator<=(const _Packed_iterator_base& __i) const
      { return !(__i < *this); }

    bool
      operator>=(const _Packed_iterator_base& __i) const
      { return !(*this < __i); }
  };

  inline ptrdiff_t
    operator-(const _Packed_iterator_base& __x, const _Packed_iterator_base& __y)
    {
      return (int(_S_word_bit) * (__x._M_p - __y._M_p)
          + __x._M_offset - __y._M_offset);
    }

  struct _Packed_iterator : public _Packed_iterator_base
  {
    typedef _Bit_reference  reference;
    typedef _Bit_reference* pointer;
    typedef _Packed_iterator   iterator;

    _Packed_iterator() : _Packed_iterator_base(0, 0) { }

    _Packed_iterator(_Storage_type * __x, unsigned int __y)
      : _Packed_iterator_base(__x, __y) { }

    iterator
      _M_const_cast() const
      { return *this; }

    reference
      operator*() const
      { return reference(_M_p, 1UL << _M_offset); }

    iterator&
      operator++()
      {
        _M_bump_up();
        return *this;
      }

    iterator
      operator++(int)
      {
        iterator __tmp = *this;
        _M_bump_up();
        return __tmp;
      }

    iterator&
      operator--()
      {
        _M_bump_down();
        return *this;
      }

    iterator
      operator--(int)
      {
        iterator __tmp = *this;
        _M_bump_down();
        return __tmp;
      }

    iterator&
      operator+=(difference_type __i)
      {
        _M_incr(__i);
        return *this;
      }

    iterator&
      operator-=(difference_type __i)
      {
        *this += -__i;
        return *this;
      }

    iterator
      operator+(difference_type __i) const
      {
        iterator __tmp = *this;
        return __tmp += __i;
      }

    iterator
      operator-(difference_type __i) const
      {
        iterator __tmp = *this;
        return __tmp -= __i;
      }

    reference
      operator[](difference_type __i) const
      { return *(*this + __i); }
  };

  inline _Packed_iterator
    operator+(ptrdiff_t __n, const _Packed_iterator& __x)
    { return __x + __n; }

  struct _Bit_const_iterator : public _Packed_iterator_base
  {
    typedef bool                 reference;
    typedef bool                 const_reference;
    typedef const bool*          pointer;
    typedef _Bit_const_iterator  const_iterator;

    _Bit_const_iterator() : _Packed_iterator_base(0, 0) { }

    _Bit_const_iterator(_Storage_type * __x, unsigned int __y)
      : _Packed_iterator_base(__x, __y) { }

    _Bit_const_iterator(const _Packed_iterator& __x)
      : _Packed_iterator_base(__x._M_p, __x._M_offset) { }

    _Packed_iterator
      _M_const_cast() const
      { return _Packed_iterator(_M_p, _M_offset); }

    const_reference
      operator*() const
      { return _Bit_reference(_M_p, 1UL << _M_offset); }

    const_iterator&
      operator++()
      {
        _M_bump_up();
        return *this;
      }

    const_iterator
      operator++(int)
      {
        const_iterator __tmp = *this;
        _M_bump_up();
        return __tmp;
      }

    const_iterator&
      operator--()
      {
        _M_bump_down();
        return *this;
      }

    const_iterator
      operator--(int)
      {
        const_iterator __tmp = *this;
        _M_bump_down();
        return __tmp;
      }

    const_iterator&
      operator+=(difference_type __i)
      {
        _M_incr(__i);
        return *this;
      }

    const_iterator&
      operator-=(difference_type __i)
      {
        *this += -__i;
        return *this;
      }

    const_iterator
      operator+(difference_type __i) const
      {
        const_iterator __tmp = *this;
        return __tmp += __i;
      }

    const_iterator
      operator-(difference_type __i) const
      {
        const_iterator __tmp = *this;
        return __tmp -= __i;
      }

    const_reference
      operator[](difference_type __i) const
      { return *(*this + __i); }
  };





#endif
