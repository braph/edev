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
}

void PackedVector :: resize(size_t n, value_type value) {
  __enter__("%lu, %d", n, value);

  reserve(n);
  while (_size < n)
    set(_size++, value);
}

void PackedVector :: push_back(value_type value) {
  __enter__("%d", value);

  if (_size == _capacity)
    reserve(_size * GROW_FACTOR + 1);

  set(_size, value);
  ++_size;
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
}

void DynamicPackedVector :: resize(size_t n, value_type value) {
  __enter__("%lu, %d", n, value);

  reserve(n, bitlength(value));
  while (_vec._size < n)
    _vec.set(_vec._size++, value);
}

void DynamicPackedVector :: set(size_t index, value_type value) noexcept {
  __enter__("index = %lu, value = %d", index, value);

  int value_bits = bitlength(value);
  if (value_bits > _vec.bits())
    _vec = PackedVector(value_bits, _vec.capacity(), _vec.begin(), _vec.end());
  _vec.set(index, value);
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

