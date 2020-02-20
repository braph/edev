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
  //std::cerr << "reserve(" << n << ")" << std::endl;
  if (n > capacity())
    *this = PackedVector(_bits, n, begin(), end());
}

void PackedVector :: resize(size_t n, int value) {
  //std::cerr << "resize(" << n << ")" << std::endl;
  reserve(n);
  while (_size < n)
    push_back(value);
}

void PackedVector :: push_back(const int &value) {
  //std::cerr << "push_back(" << value << ")" << std::endl;
  if (_size == capacity())
    reserve(_size * 2 + 1); // XXX

  set(_size, value);
  ++_size;
}

PackedVector::value_type PackedVector :: get(size_t index) const {
  //std::cerr << "get(" << index << ")" << std::endl;

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

#if TEST_PACKEDVECTOR
#include "test.hpp"
int main() {
  TEST_BEGIN();
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

  {
    DynamicPackedVector v;
    for (int i = 0; i < 256; ++i) {
      v.push_back(i);
      assert(v[i] == i);
      std::cerr << v.bits() << std::endl;
    }
  }


  TEST_END();
}
#endif
