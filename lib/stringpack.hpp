#ifndef LIB_STRINGPACK_HPP
#define LIB_STRINGPACK_HPP

// Stolen and adapted from https://github.com/alipha/cpp/blob/master/switch_pack/switch_pack.h
//
// Notes:
//  - We return the value `1` as a sentinel for unmatched characters.
//    That's why every converter function adds `1` to its result.
//
// TODO: add  7bit-ascii pack (enables 1 more char) ?

#include <cstddef>
#include <cstdint>
#include <string>

/**
 * Pack strings into uint64_t.
 */

namespace StringPack {

using std::size_t;
using std::uint8_t;
using std::uint64_t;
using conv_func = uint8_t(*)(uint8_t);
enum : uint64_t { overflow = ~uint64_t(0) };

// ============================================================================
// Helper
// ============================================================================

template<class T> constexpr int bitlength(T n)
{ return (n ? 1 + bitlength(n >> 1) : 0); }

/**
 * Information about converter functions.
 */
template<conv_func conv>
struct conv_info {
  static constexpr uint8_t max_bitlength() {
    return bitlength(max_bitlength_impl(1));
  }

  static constexpr size_t shift_count() {
    return max_bitlength();
  }

  static constexpr size_t max_strlen() {
    return 64 / max_bitlength();
  }

private:
  static constexpr uint8_t max_bitlength_impl(int i) {
    return (i > 255 ? 0 : (conv(i) | max_bitlength_impl(i + 1)));
  }
};

/**
 * Compiletime pack implementation.
 *
 * Pack each character of string `s` by applying `conv` to it and shifting
 * it by `bit_shift`.
 *
 * If length `N` of string `s` exceeds `max_length` it will not compile.
 */
template<
  conv_func conv,
  size_t N,
  size_t bit_shift  = conv_info<conv>::shift_count(),
  size_t max_length = conv_info<conv>::max_strlen()
>
constexpr uint64_t pack_compiletime(const char (&s)[N], size_t i) {
  static_assert(bit_shift  == conv_info<conv>::shift_count(), "DO NOT SET THIS TEMPLATE PARAMETER");
  static_assert(max_length == conv_info<conv>::max_strlen(),  "DO NET SET THIS TEMPLATE PARAMETER");
  static_assert(N - 1 <= max_length, "String exceeds maximum length holdable by the integer type");

  return
    (i < N - 1) ? (
      pack_compiletime<conv>(s, i + 1) |
      uint64_t(conv(uint8_t(s[i]))) << (i * bit_shift)
    ) : (
      0
    );
}

/**
 * Runtime pack implementation.
 *
 * Returns the packed chars of `s` as an integer or `overflow` if the
 * input string exceeded the maximum length.
 */
template<conv_func conv >
uint64_t pack_runtime(const char* s) noexcept {
  enum : size_t {
    bit_shift  = conv_info<conv>::shift_count(),
    max_length = conv_info<conv>::max_strlen()
  };

  uint64_t result = 0;

  for (size_t i = 0; i < max_length && *s; ++i)
    result |= uint64_t(conv(uint8_t(*s++))) << (i * bit_shift);

  return *s ? overflow : result;
}

template<conv_func conv >
uint64_t pack_runtime(const char* s, size_t len) noexcept {
  enum : size_t {
    bit_shift  = conv_info<conv>::shift_count(),
    max_length = conv_info<conv>::max_strlen()
  };

  if (len > max_length)
    return overflow;

  uint64_t result = 0;
  while (len) {
    --len;
    result |= uint64_t(conv(uint8_t(s[len]))) << (len * bit_shift);
  }

  return result;
}

// ============================================================================
// Building blocks for creating sets of possible `character sets`
// ============================================================================

/// Include a single character
template< uint8_t Char, conv_func next>
inline constexpr uint8_t character(uint8_t c) {
  return 1 + (c == Char ? 0 : next(c));
}

/// Include a range of characters
template< uint8_t from, uint8_t to, conv_func next>
inline constexpr uint8_t range(uint8_t c) {
  return 1 + (c >= from && c <= to ? c - from : to - from + next(c));
}

/// Include 0-9
template<conv_func next>
inline constexpr uint8_t numeric(uint8_t c) {
  return range<'0', '9', next>(c);
}

/// Include a-z
template<conv_func next>
inline constexpr uint8_t lower(uint8_t c) {
  return range<'a', 'z', next>(c);
}

/// Include A-Z
template<conv_func next>
inline constexpr uint8_t upper(uint8_t c) {
  return range<'A', 'Z', next>(c);
}

/// Include a-zA-Z
template<conv_func next>
inline constexpr uint8_t alpha(uint8_t c) {
  return lower<upper<next>>(c);
}

/// Include a-zA-Z (case insensitive)
template<conv_func next>
inline constexpr uint8_t alpha_nocase(uint8_t c) {
  return 1 + (
    c >= 'a' && c <= 'z' ? c - 'a' :
    c >= 'A' && c <= 'Z' ? c - 'A' :
    'z' - 'a' + next(c)
  );
}

/// Use all characters (0-255)
inline constexpr uint8_t all(uint8_t c) noexcept {
  return c;
}

/// Marker for unmatched characters
inline constexpr uint8_t unmatched(uint8_t) {
  return 1;
}

template<conv_func conv >
struct packer {
  // pack() --- compile time ==================================================
  // This is a close as we can get to ensure this function is only used on string literals.

  template<size_t N>
  static inline constexpr uint64_t pack(const char (&s)[N]) noexcept
  { return pack_compiletime<conv, N>(s, 0); }

  template<size_t N>
  static inline constexpr uint64_t pack(char (&s)[N]) noexcept
  { static_assert(N&&0, "Please use `pack_runtime` for non-const char"); return 0; }

  // pack_runtime() ===========================================================
  static inline uint64_t pack_runtime(const char* s) noexcept
  { return StringPack::pack_runtime<conv>(s); }

  static inline uint64_t pack_runtime(const unsigned char* s) noexcept
  { return StringPack::pack_runtime<conv>(reinterpret_cast<const char*>(s)); }

  static inline uint64_t pack_runtime(const char* s, size_t len) noexcept
  { return StringPack::pack_runtime<conv>(s, len); }

  static inline uint64_t pack_runtime(const unsigned char* s, size_t len) noexcept
  { return StringPack::pack_runtime<conv>(reinterpret_cast<const char*>(s), len); }

  static inline uint64_t pack_runtime(const std::string& s) noexcept
  { return StringPack::pack_runtime<conv>(s.c_str(), s.size()); }

  // packer() =================================================================
  uint64_t _s;

  template<size_t N>
  constexpr packer(const char (&s)[N])
    : _s(pack_compiletime<conv, N>(s, 0))
  {}

  template<size_t N>
  constexpr packer(char (&s)[N])
    : _s(0)
  {
    static_assert(N&&0, "Please use `pack_runtime` for non-const char");
  }

  constexpr inline operator uint64_t() noexcept {
    return _s;
  }

  // Information ==============================================================
  static inline constexpr size_t max_size() noexcept
  { return conv_info<conv>::max_strlen(); }

  static inline constexpr int bit_shift() noexcept
  { return conv_info<conv>::shift_count(); }
};

using Generic     = packer<all>;
using Lower       = packer<lower<character<'_', unmatched>>>;
using Upper       = packer<upper<character<'_', unmatched>>>;
using Numeric     = packer<numeric<unmatched>>;
using Alpha       = packer<alpha<character<'_', unmatched>>>;
using AlphaNoCase = packer<alpha_nocase<character<' ', unmatched>>>;
using Alnum       = packer<numeric<alpha<character<'_', unmatched>>>>;
using AlnumNoCase = packer<numeric<alpha_nocase<character<'_', unmatched>>>>;

} // namespace StringPack

#endif
