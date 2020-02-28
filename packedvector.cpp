#include "packedvector.hpp"

#define GROW_FACTOR 2

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
    reserve(_size * GROW_FACTOR + 1);

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
    _data[dataIndex] = replace_bits<uint32_t>(e, value, bitOffset, _bits);
  } else {
    BitShiftHelper store;
    store.u32.u1 = _data[dataIndex];
    store.u32.u2 = _data[dataIndex+1];
    store.u64 = replace_bits<uint64_t>(store.u64, value, bitOffset, _bits);
    _data[dataIndex] = store.u32.u1;
    _data[dataIndex+1] = store.u32.u2;
  }
}

/* ============================================================================
 * DynamicPackedVector
 * ==========================================================================*/

void DynamicPackedVector :: reserve(size_t n, int bits) {
  __enter__("n = %lu, bits = %d", n, bits);

  if (n < _vec.size())
    n = _vec.size();

  if (bits > _vec.bits())
    _vec = PackedVector(bits, n, _vec.begin(), _vec.end());
  else
    _vec.reserve(n);

  __leave__();
}

void DynamicPackedVector :: set(size_t index, value_type value) {
  __enter__("index = %lu, value = %d", index, value);

  reserve(capacity(), bitlength_32(value));
  _vec.set(index, value);

  __leave__();
}

void DynamicPackedVector :: push_back(value_type value) {
  __enter__("%d", value);

  size_t new_size;
  if (size() == capacity())
    new_size = size() * GROW_FACTOR + 1;
  else
    new_size = capacity(); // keep capacity!

  reserve(new_size, bitlength_32(value));
  _vec.push_back(value);

  __leave__();
}

void DynamicPackedVector :: resize(size_t n, value_type value) {
  __enter__("%lu, %d", n, value);

  reserve(n, bitlength_32(value));
  _vec.resize(n, value);

  __leave__();
}

void DynamicPackedVector :: shrink_to_fit() {
  value_type max = 0;
  for (auto e : _vec)
    if (e > max)
      max = e;

  int max_bits = bitlength_32(max);
  if (max_bits != 0 && max_bits < _vec.bits())
    _vec = PackedVector(max_bits, size(), _vec.begin(), _vec.end());
}

#ifdef TEST_PACKEDVECTOR
#include "test.hpp"
#include <vector>
#include <initializer_list>

template<typename TInt>
void testCompress(const std::initializer_list<TInt>& list, unsigned bitwidth) {
  std::vector<TInt> expected(list.begin(), list.end());
  std::sort(expected.begin(), expected.end(), std::greater<TInt>());

  TInt compressed = compress<TInt>(list.begin(), list.end(), bitwidth);
  std::vector<TInt> uncompressed;

  uncompress<TInt, std::vector<TInt>>(uncompressed, compressed, bitwidth);

  if (expected != uncompressed)
    throw std::runtime_error("FOO");
}

int main() {
  TEST_BEGIN();

  // ==========================================================================
  {
    testCompress<uint8_t>({1}, 5);
    testCompress<uint8_t>({1,2}, 5);
    testCompress<uint16_t>({3,6,12,10}, 5);
  }

  // Test: 32bit
  {
    TinyPackedArray<32, uint32_t> array;
    assert(array.capacity() == 1);
    array.push_back(0xBEEF);
    assert(array[0] == 0xBEEF);
  }

  // Test: 24bit
  {
    TinyPackedArray<24, uint32_t> array;
    assert(array.capacity() == 1);
    array.push_back(0xBEEF);
    assert(array[0] == 0xBEEF);
  }

  // Test: 16bit
  {
    TinyPackedArray<16, uint32_t> array;
    assert(array.capacity() == 2);
    array.push_back(0xDEAD);
    array.push_back(0xBEEF);
    assert(array[0] == 0xDEAD);
    assert(array[1] == 0xBEEF);
  }

  // Test: 8bit
  {
    TinyPackedArray<8, uint32_t> array;
    assert(array.capacity() == 4);
    array.push_back(0xAA);
    array.push_back(0xBB);
    array.push_back(0xCC);
    array.push_back(0xDD);
    assert(array[0] == 0xAA);
    assert(array[1] == 0xBB);
    assert(array[2] == 0xCC);
    assert(array[3] == 0xDD);
  }

  // Test: 3bit
  {
    TinyPackedArray<3, uint32_t> arr;
    assert(arr.capacity() == 10);
    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);
    arr.push_back(4);
    arr.push_back(5);
    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);
    arr.push_back(4);
    arr.push_back(5);
    assert(arr[0] == 1 && arr[1] == 2 && arr[2] == 3 && arr[3] == 4 && arr[4] == 5);
    assert(arr[5] == 1 && arr[6] == 2 && arr[7] == 3 && arr[8] == 4 && arr[9] == 5);

    size_t sum = 0;
    for (auto i : arr) { sum += i; }
    assert(sum == 30);
  }

  // Test: Access
  {
    TinyPackedArray<3, uint32_t> arr;
    arr.push_back(0);
    arr.push_back(1);
    arr[0] = 4;
    assert(arr[0] == 4);
    arr[1] = arr[0];
    assert(arr[1] == 4);
  }

  // Test: Sorting
  {
    TinyPackedArray<3, uint32_t> arr;
    arr.push_back(5);
    arr.push_back(2);
    arr.push_back(1);
    arr.push_back(4);
    arr.push_back(3);

    std::swap(*(arr.begin()), *(arr.begin()+1));
    std::cout << *(arr.begin()) << ' ' << *(arr.begin()+1) << std::endl;


#if 1
    std::sort(arr.begin(), arr.end());
    for (auto i : arr) { std::cout << i << std::endl; }
    assert(arr[0] == 1 && arr[1] == 2 && arr[2] == 3 && arr[3] == 4 && arr[4] == 5);
#endif
  }

  // Test: Sorting
  {
    TinyPackedArray<5, uint32_t> arr;
    arr.push_back(5);
    arr.push_back(2);
    arr.push_back(1);

    std::sort(arr.begin(), arr.end(), std::greater<uint8_t>());
    std::cout << arr.data() << std::endl;

    std::sort(arr.begin(), arr.end(), std::less<uint8_t>());
    std::cout << arr.data() << std::endl;
  }

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
