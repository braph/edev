#include "packedvector.hpp"
#include "packed/packed_traits.hpp"

#include <cstring>

#define GROW_FACTOR 2

/* ============================================================================
 * PackedVector
 * ==========================================================================*/

void PackedVector :: reserve(size_t n) {
  LIB_PACKEDVECTOR_TRACE("%lu", n);

  if (n > _capacity) {
    data_type* new_data = new data_type[size_for_bits(_bits * n, sizeof(data_type))];
    std::memcpy(new_data, _data, size_for_bits(_bits * _size, 1));
    delete[] _data;
    _data = new_data;
    _capacity = n;
  }
}

void PackedVector :: resize(size_t n, value_type value) {
  LIB_PACKEDVECTOR_TRACE("%lu, %d", n, value);

  reserve(n);
  while (_size < n)
    set(_size++, value);
}

void PackedVector :: push_back(value_type value) {
  LIB_PACKEDVECTOR_TRACE("%d", value);

  if (_size == _capacity)
    reserve(_size * GROW_FACTOR + 1);

  set(_size, value);
  ++_size;
}

PackedVector::value_type PackedVector :: get(size_t index) const noexcept {
  return packed_traits<data_type, value_type>::get(_data, _bits, index);
}

void PackedVector :: set(size_t index, value_type value) noexcept {
  packed_traits<data_type, value_type>::set(_data, _bits, index, value);
}

/* ============================================================================
 * DynamicPackedVector
 * ==========================================================================*/

void DynamicPackedVector :: reserve(size_t n, int bits) {
  LIB_PACKEDVECTOR_TRACE("n = %lu, bits = %d", n, bits);

  if (bits > _vec.bits()) {
    if (n < _vec.capacity())
      n = _vec.capacity();
    _vec = PackedVector(bits, n, _vec.begin(), _vec.end());
  }
  else
    _vec.reserve(n);
}

void DynamicPackedVector :: resize(size_t n, value_type value) {
  LIB_PACKEDVECTOR_TRACE("%lu, %d", n, value);

  reserve(n, bitlength(value));
  while (_vec._size < n)
    _vec.set(_vec._size++, value);
}

void DynamicPackedVector :: set(size_t index, value_type value) noexcept {
  LIB_PACKEDVECTOR_TRACE("index = %lu, value = %d", index, value);

  int value_bits = bitlength(value);
  if (value_bits > _vec.bits())
    _vec = PackedVector(value_bits, _vec.capacity(), _vec.begin(), _vec.end());
  _vec.set(index, value);
}

void DynamicPackedVector :: push_back(value_type value) {
  LIB_PACKEDVECTOR_TRACE("%d", value);

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

