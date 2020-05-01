#include "packedvector.hpp"

#include <cstring>

#define GROW_FACTOR 2

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

  if (n > _capacity) {
    data_type* new_data = new data_type[size_for_bits(_bits * n, sizeof(data_type))];
    std::memcpy(new_data, _data, size_for_bits(_bits * _size, 1));
    delete[] _data;
    _data = new_data;
    _capacity = n;
  }

  __leave__();
}

void PackedVector :: resize(size_t n, value_type value) {
  __enter__("%lu, %d", n, value);

  reserve(n);
  while (_size < n)
    set(_size++, value);

  __leave__();
}

void PackedVector :: push_back(value_type value) {
  __enter__("%d", value);

  if (_size == _capacity)
    reserve(_size * GROW_FACTOR + 1);

  set(_size, value);
  ++_size;

  __leave__();
}

#define USE_POSSIBLE_FASTER_IMPLEMENTATION 0
PackedVector::value_type PackedVector :: get(size_t index) const noexcept {
  unsigned int dataIndex = (index * _bits) / 32;
  unsigned int bitOffset = (index * _bits) % 32;

#if ! USE_POSSIBLE_FASTER_IMPLEMENTATION
  if (bitOffset + _bits <= 32) {
    uint32_t e = _data[dataIndex];
    e = (e >> bitOffset) &~(0xFFFFFFFFu << _bits);
    return value_type(e);
  } else {
#endif
    BitShiftHelper store;
    store.u32.u1 = _data[dataIndex];
    store.u32.u2 = _data[dataIndex+1];
    uint32_t e = (store.u64 >> bitOffset) &~(0xFFFFFFFFu << _bits);
    return value_type(e);
#if ! USE_POSSIBLE_FASTER_IMPLEMENTATION
}
#endif
}

void PackedVector :: set(size_t index, value_type value) noexcept {
  unsigned int dataIndex = (index * _bits) / 32;
  unsigned int bitOffset = (index * _bits) % 32;

  if (bitOffset + _bits <= 32) {
    uint32_t e = _data[dataIndex];
    _data[dataIndex] = replace_bits<uint32_t>(e, uint32_t(value), int(bitOffset), int(_bits));
  } else {
    BitShiftHelper store;
    store.u32.u1 = _data[dataIndex];
    store.u32.u2 = _data[dataIndex+1];
    store.u64 = replace_bits<uint64_t>(store.u64, uint32_t(value), int(bitOffset), int(_bits));
    _data[dataIndex] = store.u32.u1;
    _data[dataIndex+1] = store.u32.u2;
  }
}

/* ============================================================================
 * DynamicPackedVector
 * ==========================================================================*/

void DynamicPackedVector :: reserve(size_t n, int bits) {
  __enter__("n = %lu, bits = %d", n, bits);

  if (bits > _vec.bits()) {
    if (n < _vec.capacity())
      n = _vec.capacity();
    _vec = PackedVector(bits, n, _vec.begin(), _vec.end());
  }
  else
    _vec.reserve(n);

  __leave__();
}

void DynamicPackedVector :: resize(size_t n, value_type value) {
  __enter__("%lu, %d", n, value);

  reserve(n, bitlength(value));
  while (_vec._size < n)
    _vec.set(_vec._size++, value);

  __leave__();
}

void DynamicPackedVector :: set(size_t index, value_type value) noexcept {
  __enter__("index = %lu, value = %d", index, value);

  int value_bits = bitlength(value);
  if (value_bits > _vec.bits())
    _vec = PackedVector(value_bits, _vec.capacity(), _vec.begin(), _vec.end());
  _vec.set(index, value);

  __leave__();
}

void DynamicPackedVector :: push_back(value_type value) {
  __enter__("%d", value);

  int value_bits = bitlength(value);
  if (value_bits > _vec.bits())
    // We have to recreate the vector if the bitwidth has increased.
    // To save a conditional branch we don't check if the capacity needs to
    // be changed. Instead we add +1 to the capacity to ensure we have space
    // for the new element.
    _vec = PackedVector(value_bits, _vec._capacity + 1, _vec.begin(), _vec.end());
  else if (_vec._capacity == _vec._size)
    _vec.reserve((_vec._capacity + 1) * GROW_FACTOR);

  _vec.set(_vec._size, value);
  ++_vec._size;

  __leave__();
}

void DynamicPackedVector :: shrink_to_fit() {
  value_type max = 0;
  for (auto e : _vec)
    if (e > max)
      max = e;

  int max_bits = bitlength(max);
  if (max_bits != 0 && max_bits < _vec.bits())
    _vec = PackedVector(max_bits, size(), _vec.begin(), _vec.end());
}

#ifdef TEST_PACKEDVECTOR
#include "test.hpp"
#include <vector>
#include <initializer_list>

/**
 * Foo
 */
template<typename TValue, typename TTestee, typename TExpect>
struct VectorTester {
  using value_type = TValue;

  TTestee testee; // The vector TO be tested
  TExpect expect; // The reference vector

#define proxy0(F_RET, F_NAME) \
  F_RET F_NAME() { \
    testee.F_NAME(); \
    expect.F_NAME(); }

#define proxy1(F_RET, F_NAME, A1_TYPE, A1_NAME) \
  F_RET F_NAME(A1_TYPE A1_NAME) { \
    testee.F_NAME(A1_NAME); \
    expect.F_NAME(A1_NAME); }

  proxy0(void,clear)
  proxy0(void,pop_back)
  proxy1(void,push_back,value_type,v)
  proxy1(void,reserve,size_t,n)
  proxy1(void,resize,size_t,n)

  struct reference {
    TTestee& testee;
    TExpect& expect;
    size_t pos;

    reference(TTestee& testee, TExpect &expect, size_t pos)
      : testee(testee)
      , expect(expect)
      , pos(pos)
    {}

    template<typename TRhsValue>
    reference& operator=(const TRhsValue& v) {
      testee[pos] = v;
      expect[pos] = v;
      return *this;
    }
  };

  reference operator[](size_t pos) {
    return reference(testee, expect, pos);
  }

#define _check(...) \
  if (! (__VA_ARGS__)) throw std::runtime_error(#__VA_ARGS__)

  void check_empty()    { _check( testee.empty()    == expect.empty() ); }
  void check_size()     { _check( testee.size()     == expect.size()  ); }
  void check_front()    { _check( testee.front()    == expect.front() ); }
  void check_back()     { _check( testee.back()     == expect.back()  ); }
  void check_capacity() { _check( testee.capacity() == expect.capacity() ); }
  void check_equals_using_index_access() {
    size_t sz = expect.size();
    for (size_t i = 0; i < sz; ++i)
      _check( testee[i] == expect[i] );
  }
  void check_equals_using_iterator_access() {
    auto testee_it = testee.begin(), testee_end = testee.end();
    auto expect_it = expect.begin(), expect_end = expect.end();

    while (testee_it != testee_end && expect_it != expect_end)
      _check( *testee_it++ == *expect_it++ );

    _check( testee_it == testee_end );
    _check( expect_it == expect_end );
  }

  void check_all() {
    check_empty();
    check_size();
  }
};

int main() {
  TEST_BEGIN();

  using V = VectorTester<int, std::vector<int>, DynamicPackedVector>;

  {
    V v;
    v.check_all();
    assert(v.testee.capacity() == 0);

    // Basic
    for (int i = 0; i < 1024; ++i)
      v.push_back(i);
    v.check_all();

    // Clear
    v.clear();
    v.check_all();

    // Foo
    for (int i = 0; i <= INT_MAX; i *= 2)
      v.push_back(i);
    v.check_all();

    // Direct access (get)
    for (int i = 0) {}

    // Direct access (set)
    for (int i = 0; i < TODO; ++i)
      v[i] = 2;
    v.check_all();

  }

#define SZ 1048576
  {
    std::cerr << "*iterator =\n";
    for (DynamicPackedVector::iterator it = v.begin(); it != v.end(); ++it) {
      *it = 33;
    }

    std::cerr << "get[33]\n";
    for (size_t i = 0; i < SZ; ++i) {
      assert(v[i] == 33);
    }
  }

  TEST_END();
}
#endif
