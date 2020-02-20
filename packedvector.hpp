#ifndef _PACKED_VECTOR_HPP
#define _PACKED_VECTOR_HPP

#include "common.hpp"
#include "generic.hpp"

#include <new>
#include <memory>
#include <iostream> //XXX

static inline uint32_t bitlength_32(uint32_t n) {
  return (n ? 32 - __builtin_clz(n) : 0);
}

class PackedVector {
private:
  uint32_t* _data;
  size_t    _size;
  size_t    _capacity;
  uint8_t   _bits;

public:
  typedef GenericIterator<PackedVector> iterator;
  typedef GenericReference<PackedVector> reference;
  typedef int value_type;
  typedef uint32_t storage_type;

  friend class DynamicPackedVector;

  PackedVector(int bits)
  : _data(NULL)
  , _size(0)
  , _capacity(0)
  , _bits(bits) {
    if (bits > 31 || bits < 1)
      throw std::invalid_argument("bits > 31 || bits < 1");
  }

  PackedVector(PackedVector&& rhs)
  : _data(rhs._data)
  , _size(rhs._size)
  , _capacity(rhs._capacity)
  , _bits(rhs._bits) {
    rhs._data = NULL;
    rhs._size = 0;
    rhs._capacity = 0;
  }

  ~PackedVector() {
    if (_data)
      delete[] _data;
  }

  PackedVector& operator=(PackedVector&& rhs) {
    std::swap(_data, rhs._data);
    std::swap(_size, rhs._size);
    std::swap(_capacity, rhs._capacity);
    _bits = rhs._bits;
    return *this;
  }

  reference operator[](size_t idx) { return reference(*this, idx);    }
  iterator  begin()                { return iterator(*this, 0);       }
  iterator  end()                  { return iterator(*this, size());  }

  bool    empty()     const   { return _size == 0;                }
  size_t  size()      const   { return _size;                     }
  size_t  capacity()  const   { return _capacity * 32 / _bits;    }
  void    clear()             { _size = 0;                        } 
  void*   data()              { return static_cast<void*>(_data); }
  
  void        push_back(const int &value_type);
  void        reserve(size_t);
  void        resize(size_t n, int value_type = 0);

  // Methods unique to PackedVector
  void        set(size_t, value_type);
  value_type  get(size_t) const;
  int         bits()      const   { return _bits;                       }
  size_t      data_size() const   { return bytes_for_bits(_bits*_size); }

protected:
  PackedVector(int bits, size_t capacity, iterator begIt, iterator endIt)
  : _data(new storage_type[bytes_for_bits(bits*capacity) / sizeof(storage_type) + 1])
  , _size(0) // It's not yet filled!
  , _capacity(bytes_for_bits(bits*capacity) / sizeof(storage_type) + 1)
  , _bits(bits)
  {
    //std::cerr << "private PackedVector(" << bits << "," << capacity << "," << begIt << "," << endIt << ")" << std::endl;
    while (begIt != endIt)
      push_back(*begIt++);
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

  reference operator[](size_t index)  { return reference(*this, index); }
  iterator  begin()                   { return iterator(*this, 0);      }
  iterator  end()                     { return iterator(*this, size()); }

  // Proxy methods to PackedVector ============================================
  bool    empty()            const   { return _vec.empty();                   }
  size_t  size()             const   { return _vec.size();                    }
  size_t  capacity()         const   { return _vec.capacity();                }
  void    clear()                    { _vec.clear();                          }
  void*   data()                     { return _vec.data();                    }
  int     bits()             const   { return _vec.bits();                    }
  size_t  data_size()        const   { return _vec.data_size();               }
  value_type get(size_t idx) const   { return _vec.get(idx);                  }
  void    reserve(size_t n)          { _vec.reserve(n);                       }

  /* Customized functions that increases the bitwidth of the underlying
   * PackedVector if needed */

  void set(size_t index, value_type value = 0) {
    int bits = bitlength_32(value);
    if (bits > _vec.bits())
      _vec = PackedVector(bits, _vec.size(), _vec.begin(), _vec.end());
    _vec.set(index, value);
  }

  void push_back(const value_type& value) {
    int bits = bitlength_32(value);
    if (bits > _vec.bits())
      _vec = PackedVector(bits, _vec.size() + 1, _vec.begin(), _vec.end());
    _vec.push_back(value);
  }

  void resize(size_t n, value_type value = 0) {
    int bits = bitlength_32(value);
    if (bits > _vec.bits())
      _vec = PackedVector(bits, n, _vec.begin(), _vec.end());
    _vec.resize(n, value);
  }
};

#endif

