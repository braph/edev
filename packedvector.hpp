#ifndef _PACKED_VECTOR_HPP
#define _PACKED_VECTOR_HPP

#include "common.hpp"
#include "generic.hpp"

#include <new>
#include <memory>
#include <iostream> //XXX

#define DEBUG_VECTOR 0
#if DEBUG_VECTOR
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

static inline uint32_t bitlength_32(uint32_t n) {
  return (n ? 32 - __builtin_clz(n) : 0);
}

class PackedVector {
private:
  typedef uint32_t storage_type;

  storage_type* _data;
  size_t  _size;     // element count
  size_t  _capacity; // element count
  uint8_t _bits;

public:
  typedef GenericIterator<PackedVector> iterator;
  typedef GenericReference<PackedVector> reference;
  typedef int value_type;

  friend class DynamicPackedVector;

  PackedVector(int bits)
  : _data(NULL)
  , _size(0)
  , _capacity(0)
  , _bits(bits) {
    __enter__("bits = %d", bits);
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

  size_t  size()      const   { return _size;                     }
  bool    empty()     const   { return _size == 0;                }
  void    clear()             { _size = 0;                        } 
  void*   data()              { return static_cast<void*>(_data); }
  size_t  capacity()  const   { return _capacity;                 }
  
  void    push_back(value_type);
  void    reserve(size_t);
  void    resize(size_t n, value_type value = 0);

  // Methods unique to PackedVector
  void        set(size_t, value_type);
  value_type  get(size_t) const;
  int         bits()      const   { return _bits;                       }
  size_t      data_size() const   { return size_for_bits(_bits*_size);  }

protected:
  PackedVector(int bits, size_t capacity, iterator begIt, iterator endIt)
  : _data(new storage_type[size_for_bits(bits*capacity, sizeof(storage_type))])
  , _size(0)
  , _capacity(capacity)
  , _bits(bits)
  {
    __enter__("%d, %lu, iterator, iterator", bits, capacity);
//std::cerr << "COPYSO" << std::endl;
    while (begIt != endIt)
      push_back(*begIt++);
//std::cerr << "/COPYSO" << std::endl;
    __leave__();
  }
};

class DynamicPackedVector {
private:
  PackedVector _vec;
public:
  DynamicPackedVector() : _vec(1) { }

  typedef GenericReference<DynamicPackedVector> reference;
  typedef GenericIterator<DynamicPackedVector> iterator;
  typedef int value_type;

  reference operator[](size_t index) { return reference(*this, index); }
  iterator  begin()                  { return iterator(*this, 0);      }
  iterator  end()                    { return iterator(*this, size()); }

  // Proxy methods to PackedVector ============================================
  void    clear()                    { _vec.clear();                   }
  size_t  size()             const   { return _vec.size();             }
  bool    empty()            const   { return _vec.empty();            }
  size_t  capacity()         const   { return _vec.capacity();         }
  void*   data()                     { return _vec.data();             }
  int     bits()             const   { return _vec.bits();             }
  size_t  data_size()        const   { return _vec.data_size();        }
  value_type get(size_t idx) const   { return _vec.get(idx);           }

  /* Customized functions that increases the bitwidth of the underlying
   * PackedVector if needed */

  void reserve(size_t n, int bits = 1) {
    __enter__("n = %lu, bits = %d", n, bits);

//std::cerr << "c=" << capacity() << std::endl;
    if (n < _vec.size())
      n = _vec.size();

    if (bits > _vec.bits())
      _vec = PackedVector(bits, n, _vec.begin(), _vec.end());
    else
      _vec.reserve(n);
//std::cerr << "c=" << capacity() << std::endl;

    __leave__();
  }

  void set(size_t index, value_type value = 0) {
    __enter__("index = %lu, value = %d", index, value);
    reserve(capacity(), bitlength_32(value));
    _vec.set(index, value);
    __leave__();
  }

  void push_back(value_type value) {
    __enter__("%d", value);

    size_t new_size;
    if (size() == capacity()) {
      new_size = size() * 1.5 + 1;
    } else {
      new_size = capacity(); // TODO...
    }

    reserve(new_size, bitlength_32(value));
    _vec.push_back(value);
    __leave__();
  }

  // resize constructor != reserve constructor TODO
  void resize(size_t n, value_type value = 0) {
    __enter__("%lu, %d", n, value);
    reserve(n, bitlength_32(value));
    _vec.resize(n, value);
    __leave__();
  }
};

#endif

