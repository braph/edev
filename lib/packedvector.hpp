#ifndef PACKED_VECTOR_HPP
#define PACKED_VECTOR_HPP

#include "genericiterator.hpp"
#include "genericreference.hpp"
#include "bit_tools.hpp"
#include "packedarray.hpp" // TODO

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
    if (_bits < 1)
      _bits = 1;
    else if (_bits > 32)
      _bits = 32;
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

