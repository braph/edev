#ifndef BIT_TOOLS_HPP
#define BIT_TOOLS_HPP

#include "staticvector.hpp"

#include <limits>
#include <climits>
#include <cstddef>
#include <type_traits>

#define BITSOF(T) (CHAR_BIT * sizeof(T))

template<typename TInt>
StaticVector<char, sizeof(TInt) * CHAR_BIT> extract_set_bits(TInt value) {
  StaticVector<char, sizeof(TInt) * CHAR_BIT> result;
  for (int i = 0; i < int(sizeof(TInt) * CHAR_BIT); ++i)
    if (value & (1 << i))
      result.push_back(i + 1);
  return result;
}

// Unsigned ===================================================================
static inline int bitlength(unsigned long long n) noexcept {
  return (!n ? 0 : int(sizeof(long long) * CHAR_BIT) - __builtin_clzll(n));
}

static inline int bitlength(unsigned long n) noexcept {
  return (!n ? 0 : int(sizeof(long) * CHAR_BIT) - __builtin_clzl(n));
}

static inline int bitlength(unsigned int n) noexcept {
  return (!n ? 0 : int(sizeof(int) * CHAR_BIT) - __builtin_clz(n));
}

static inline int bitlength(unsigned short n) noexcept {
  return (!n ? 0 : int(sizeof(int) * CHAR_BIT) - __builtin_clz(n));
}

static inline int bitlength(unsigned char n) noexcept {
  return (!n ? 0 : int(sizeof(int) * CHAR_BIT) - __builtin_clz(n));
}

// Signed =====================================================================
static inline int bitlength(long long n) noexcept {
  return bitlength(static_cast<unsigned long long>(n));
}

static inline int bitlength(long n) noexcept {
  return bitlength(static_cast<unsigned long>(n));
}

static inline int bitlength(int n) noexcept {
  return bitlength(static_cast<unsigned int>(n));
}

static inline int bitlength(short n) noexcept {
  return bitlength(static_cast<unsigned short>(n));
}

static inline int bitlength(char n) noexcept {
  return bitlength(static_cast<unsigned char>(n));
}

// ============================================================================
static inline size_t size_for_bits(size_t bits, size_t storage_size = 1) {
  storage_size *= CHAR_BIT;
  return (bits%storage_size ? bits/storage_size + 1 : bits/storage_size);
}

static inline constexpr size_t elements_fit_in_bits(size_t bits, size_t storage_bits) {
  return (storage_bits%bits ? storage_bits/bits : storage_bits/bits);
}

/**
 * Example for replace_bits<uint_16t>(18, 7, 3, 3)
 *
 * BIT_COUNT: 16
 * 0xFFFF:    11111111 11111111 -- TUIntType with all bits set
 * src:       00000000 00010010 -- Initial value that is going to be altered
 * val:       00000000 00000111 -- Value to to insert
 * offset:                 ^---------- where to put the value
 * len:                  ^------------ length of the value
 * mask:      00000000 11100000 <--- gives mask
 *
 * clearing `src` with `& ~mask`:
 *            00000000 00010010
 *            00000000 00000010
 */
template<typename TUIntType> // TODO std::make_unsigned
inline TUIntType replace_bits(TUIntType src, TUIntType val, int offset, int len) {
  enum { BIT_COUNT = CHAR_BIT * sizeof(TUIntType) };
  const TUIntType OxFFFF = std::numeric_limits<TUIntType>::max();

  // secure val to len bits
  val = val & ~(OxFFFF << len);

  // 4.52637 +- 0.00775 seconds time elapsed
  // We are replacing the whole `src`
  if (! offset && len == BIT_COUNT)
    return val;

  TUIntType mask = (~(OxFFFF << len)) << offset;
  return (src & ~mask) | (val << offset);
}

template<typename TUIntType>
inline TUIntType extract_bits(TUIntType src, int offset, int len) {
  const TUIntType OxFFFF = std::numeric_limits<TUIntType>::max();
  return (src >> offset) &~(OxFFFF << len);
}

#endif
