#ifndef PACKED_VECTOR_HPP
#define PACKED_VECTOR_HPP

#include "bit_tools.hpp"
#include "genericiterator.hpp"
#include "genericreference.hpp"
#include "packed/packed_traits.hpp"

#include <limits>
#include <cstring> // memcpy
#include <type_traits>

#ifdef LIB_PACKEDVECTOR_DEBUG
#include <cstdio>
static int call_level = 0;
struct Trace { ~Trace() { --call_level; } };
#define LIB_PACKEDVECTOR_TRACE(FMT, ...) \
  Trace trc{}; printf("%*s%s(" FMT ")\n", call_level++, "", __PRETTY_FUNCTION__, __VA_ARGS__)
#define debug(FMT, ...) printf("%*s" FMT "\n", call_level, "", __VA_ARGS__)
#else
#define LIB_PACKEDVECTOR_TRACE(...) (void)0
#define debug(...) (void)0
#endif

#define LIB_PACKEDVECTOR_GROW_FACTOR 2

/* ============================================================================
 * PackedVector
 * ==========================================================================*/

template<class T>
class PackedVector {
public:
  using value_type      = T;
  using iterator        = GenericIterator<PackedVector>;
  using const_iterator  = GenericConstIterator<PackedVector>;
  using reference       = GenericReference<PackedVector>;
  using const_reference = GenericConstReference<PackedVector>;
  using data_type       = typename std::make_unsigned<T>::type;

protected:
  data_type* _data;
  size_t     _size;     // element count
  size_t     _capacity; // element count
  data_type  _bit_mask; // bit mask representing current bit length
  uint8_t    _bits;     // number of bits

public:
  PackedVector(int bits) noexcept
  : _data(NULL)
  , _size(0)
  , _capacity(0)
  , _bit_mask(make_bitmask(clamp_bits(bits)))
  , _bits(clamp_bits(bits))
  {
    LIB_PACKEDVECTOR_TRACE("%u", bits);
  }

  PackedVector(PackedVector&& rhs) noexcept
  : _data(rhs._data)
  , _size(rhs._size)
  , _capacity(rhs._capacity)
  , _bit_mask(rhs._bit_mask)
  , _bits(rhs._bits)
  {
    LIB_PACKEDVECTOR_TRACE("%s", "rhs");
    rhs._data = NULL;
    rhs._size = 0;
    rhs._capacity = 0;
  }

 ~PackedVector() {
    delete[] _data;
  }

  PackedVector& operator=(PackedVector&& rhs) noexcept {
    LIB_PACKEDVECTOR_TRACE("%s", "rhs");
    std::swap(_data, rhs._data);
    std::swap(_size, rhs._size);
    std::swap(_capacity, rhs._capacity);
    _bit_mask = rhs._bit_mask;
    _bits = rhs._bits;
    return *this;
  }

  reference        operator[](size_t idx)       noexcept { return reference(this, idx);       }
  const_reference  operator[](size_t idx) const noexcept { return const_reference(this, idx); }

  iterator        begin()           noexcept { return iterator(this, 0);      }
  iterator        end()             noexcept { return iterator(this, size()); }
  const_iterator  begin()     const noexcept { return iterator(this, 0);      }
  const_iterator  end()       const noexcept { return iterator(this, size()); }

  size_t          size()      const noexcept { return _size;      }
  bool            empty()     const noexcept { return _size == 0; }
  void            clear()           noexcept { _size = 0;         }
  data_type*      data()            noexcept { return _data;      }
  size_t          capacity()  const noexcept { return _capacity;  }
  void            shrink_to_fit()            { /* TODO */         }
  void            emplace_back(value_type v) { push_back(v);      }

  reference       front()           noexcept { return operator[](0);          }
  reference       back()            noexcept { return operator[](size() - 1); }
  const_reference front()     const noexcept { return operator[](0);          }
  const_reference back()      const noexcept { return operator[](size() - 1); }

  // PackedVector specific methods (not available in std::vector)
  int             bits()      const noexcept { return _bits; }
  data_type       bit_mask()  const noexcept { return _bit_mask; }

  void reserve(size_t n) {
    LIB_PACKEDVECTOR_TRACE("%lu", n);

    if (n > _capacity) {
      const size_t needed_blocks = size_for_bits(_bits * n, sizeof(data_type));
      data_type* new_data = new data_type[needed_blocks];
      std::memcpy(new_data, _data, size_for_bits(_bits * _size, 1));
      delete[] _data;
      _data = new_data;
      _capacity = needed_blocks * BITSOF(data_type) / _bits;
    }
  }

  void resize(size_t n, value_type value = 0) {
    LIB_PACKEDVECTOR_TRACE("%lu, %d", n, value);

    reserve(n);
    while (_size < n)
      set(_size++, value);
  }

  void push_back(value_type value) {
    LIB_PACKEDVECTOR_TRACE("%d", value);

    if (_size == _capacity)
      reserve(_size * LIB_PACKEDVECTOR_GROW_FACTOR + 1);

    set(_size, value);
    ++_size;
  }

  value_type get(size_t index) const noexcept {
    return value_type(packed_traits<data_type>::get(_data, _bits, index));
  }

  void set(size_t index, value_type value) noexcept {
    packed_traits<data_type>::set(_data, _bits, index, data_type(value));
  }

/* protected */
  static PackedVector copy(iterator begIt, iterator endIt, size_t capacity, int bits) {
    LIB_PACKEDVECTOR_TRACE("%d", bits);
    PackedVector v(bits);
    v.reserve(capacity);
    while (begIt != endIt) {
      v.set(v._size, *begIt++);
      v._size++;
    }
    return v;
  }

protected:
  static constexpr inline data_type make_bitmask(int bits) {
    return ~(std::numeric_limits<data_type>::max() << bits);
  }

  static constexpr inline int clamp_bits(int bits) {
    return (bits < 1) ? 1 : (bits > sizeof(data_type) * CHAR_BIT) ? sizeof(data_type) * CHAR_BIT : bits;
  }
};

/* ============================================================================
 * DynamicPackedVector
 * ==========================================================================*/

template<class T>
class DynamicPackedVector {
private:
  using packed_t = PackedVector<T>;
  packed_t _vec;

public:
  DynamicPackedVector() : _vec(1) {}

  using value_type      = typename packed_t::value_type;
  using reference       = GenericReference<DynamicPackedVector>;
  using const_reference = GenericConstReference<DynamicPackedVector>;
  using iterator        = GenericIterator<DynamicPackedVector>;
  using const_iterator  = GenericConstIterator<DynamicPackedVector>;
  using data_type       = typename packed_t::data_type;

  reference       operator[](size_t idx)       noexcept { return reference(this, idx);       }
  const_reference operator[](size_t idx) const noexcept { return const_reference(this, idx); }

  iterator        begin()               noexcept { return iterator(this, 0);      }
  iterator        end()                 noexcept { return iterator(this, size()); }
  const_iterator  begin()         const noexcept { return iterator(this, 0);      }
  const_iterator  end()           const noexcept { return iterator(this, size()); }

  // Proxy methods to PackedVector ============================================
  void            clear()               noexcept { _vec.clear();             }
  size_t          size()          const noexcept { return _vec.size();       }
  bool            empty()         const noexcept { return _vec.empty();      }
  size_t          capacity()      const noexcept { return _vec.capacity();   }
  data_type*      data()                noexcept { return _vec.data();       }
  int             bits()          const noexcept { return _vec.bits();       }
  value_type      get(size_t idx) const noexcept { return _vec.get(idx);     }
  void            emplace_back(value_type v)     { push_back(v);             }

  reference       front()               noexcept { return operator[](0);          }
  reference       back()                noexcept { return operator[](size() - 1); }
  const_reference front()         const noexcept { return operator[](0);          }
  const_reference back()          const noexcept { return operator[](size() - 1); }

  // ==========================================================================
  // Following methods may need to replace the underlying vector object
  // ==========================================================================

  void reserve(size_t n, int bits = 1) {
    LIB_PACKEDVECTOR_TRACE("n = %lu, bits = %d", n, bits);

    if (bits > _vec.bits()) {
      if (n < _vec.capacity())
        n = _vec.capacity();
      _vec = packed_t::copy(_vec.begin(), _vec.end(), n, bits);
    }
    else
      _vec.reserve(n);
  }

  void resize(size_t n, value_type value = 0) {
    LIB_PACKEDVECTOR_TRACE("%lu, %d", n, value);

    reserve(n, bitlength(value));
    _vec.resize(n, value);
  }

  void set(size_t index, value_type value) noexcept {
    LIB_PACKEDVECTOR_TRACE("index = %lu, value = %d", index, value);

    if (data_type(value) > _vec.bit_mask()) {
      _vec = packed_t::copy(_vec.begin(), _vec.end(), _vec.capacity(), bitlength(value));
    }

    _vec.set(index, value);
  }

  void push_back(value_type value) {
    LIB_PACKEDVECTOR_TRACE("%d", value);

    if (data_type(value) > _vec.bit_mask()) {
      // capacity + 1 to ensure that the following push_back doesn't need
      // to reallocate the again.
      _vec = packed_t::copy(_vec.begin(), _vec.end(), _vec.capacity() + 1, bitlength(value));
    }

    _vec.push_back(value);
  }

  void shrink_to_fit() {
    value_type max = 0;
    for (auto e : _vec)
      max |= e;

    int bits = bitlength(max);
    if (bits < _vec.bits())
      _vec = packed_t::copy(_vec.begin(), _vec.end(), _vec.capacity(), bits);
  }
};

#endif
