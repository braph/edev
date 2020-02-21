#include "packedvector.hpp"

/* ============================================================================
 * Static helper functions
 * ==========================================================================*/

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

/* ============================================================================
 * PackedVector
 * ==========================================================================*/

void PackedVector :: reserve(size_t n) {
  __enter__("%lu", n);
  if (n > capacity())
    *this = PackedVector(_bits, n, begin(), end());
  __leave__();
}

void PackedVector :: resize(size_t n, value_type value) {
  __enter__("%lu, %d", n, value);
  reserve(n);
  while (_size < n)
    push_back(value);
  __leave__();
}

void PackedVector :: push_back(value_type value) {
  __enter__("%d", value);
  if (_size == capacity())
    reserve(_size * 2 + 1); // TODO

  set(_size, value);
  ++_size;
  __leave__();
}

PackedVector::value_type PackedVector :: get(size_t index) const {
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

/* ============================================================================
 * DynamicPackedVector
 * ==========================================================================*/

#if TEST_PACKEDVECTOR
#include "test.hpp"
#include <vector>
int main() {
  TEST_BEGIN();
#if 0
  {
    PackedVector v(8);
    assert(v.empty());
    assert(v.size() == 0);
    assert(v.capacity() == 0);

    v.push_back(42);
    assert(v[0] == 42);

    v.push_back(13);
    assert(v[1] == 13);

    v.push_back(12);
    assert(v[2] == 12);

    v.push_back(19);
    assert(v[3] == 19);
  }
#endif

#define SZ 1048576
  {
    std::vector<int> v;
    for (int i = 0; i < SZ; ++i)
      v.push_back(i);
  }

  {
    DynamicPackedVector v;
    std::cout << "push back 0" << std::endl;
    for (int i = 0; i < 20000; ++i) {
      v.push_back(i);
      //assert(v[i] == i); XXX
    }
  }

  {
    DynamicPackedVector v;
    std::cout << "push back" << std::endl;
    for (int i = 0; i < SZ; ++i) {
      v.push_back(i);
      //assert(v[i] == i); XXX
    }

    std::cout << "set" << std::endl;
    for (int i = 0; i < SZ; ++i) {
      v[i] = 2;
    }

    std::cout << "get" << std::endl;
    for (int i = 0; i < SZ; ++i) {
      assert(v[i] == 2);
    }

    std::cout << "*iterator =" << std::endl;
    for (DynamicPackedVector::iterator it = v.begin(); it != v.end(); ++it) {
      *it = 33;
    }

    std::cout << "get[33]" << std::endl;
    for (int i = 0; i < SZ; ++i) {
      assert(v[i] == 33);
    }
  }

  {
    std::cout << "push_back" << std::endl;
    DynamicPackedVector v;
    for (int i = 0; i < SZ; ++i)
      v.push_back(1);

    std::cout << "*it = 1024" << std::endl;
    for (DynamicPackedVector::iterator it = v.begin(); it != v.end(); ++it)
      *it = 1024;

    std::cout << "v[i] == 1024" << std::endl;
    for (int i = 0; i < SZ; ++i)
      assert(v[i] == 1024);
  }

  TEST_END();
}
#endif
