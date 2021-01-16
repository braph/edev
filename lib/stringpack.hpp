#ifndef LIB_STRINGPACK_HPP
#define LIB_STRINGPACK_HPP
//
// Stolen and adapted from https://github.com/alipha/cpp/blob/master/switch_pack/switch_pack.h
//
// Pack strings into a int64_t, useful for switch-case.
// ====================================================
//
// class                   | len   | comment
// StringPack::Raw         | 8     | Fastest (no conversion applied)
// StringPack::ASCII       | 9     | Fast (chars >= 128 will be converted to 127)
// StringPack::Alnum       | 9     |
// StringPack::AlnumNoCase | 10    |
// StringPack::Alpha       | 10    |
// StringPack::AlphaNoCase | 12    |
// StringPack::Upper       | 12    |
// StringPack::Lower       | 12    |
// StringPack::L33tNoCase  | 12    | Like AlnumNoCase, but 0-9 are converted to 'OLZEASGTBQ'
// StringPack::Numeric     | 16    | Better use `switch(atoi(...)) {}` instead!
//
// Ordered by performance (better first)
// =====================================
//
// 1. Raw
// 2. ASCII
// 3. Alpha, Lower, Upper, Numeric, Floatic
// 4. AlphaNoCase
// 5. Alnum
// 6. AlnumNoCase
// 7. L33tNoCase
//
// How does it work
// ================
//
// StringPack::Numeric::pack("012X")  returns  0x000000000000B321
//
// - The characters are beeing pushed in reverse order into the result value.
// - The special value `0` means `no character`
// - Each character handled by a conversion function gets enumerated.
//   The enumeration value will be used in the result: '0' -> 1, '1' -> 2, etc...
// - The total number of available characters + 2 give the `unmatched` value (10 + 2 == 12 == 0xB)
// - The `unmatched` value is needed to prevent false matches ("012X" != "012")
//
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

  static std::string characters() {
    uint8_t sentinel_ = sentinel();
    std::string r;
    for (unsigned c = 0; c < 255; ++c)
      if (conv(c) != sentinel_)
        r.push_back(c);
    return r;
  }

private:
  static constexpr uint8_t max_bitlength_impl(int i) {
    return (i > 255 ? 0 : (conv(i) | max_bitlength_impl(i + 1)));
  }

  static constexpr uint8_t sentinel(uint8_t c = 255, uint8_t return_ = 0) {
    return c == 0 ? return_ : sentinel(c-1,
      conv(c) > return_ ? conv(c) : return_);
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

  return i >= N - 1 ? 0 : (
    pack_compiletime<conv>(s, i + 1) |
    uint64_t(conv(uint8_t(s[i]))) << (i * bit_shift)
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

/// Marker for unmatched characters
inline constexpr uint8_t unmatched(uint8_t) {
  return 1;
}

/// Include a single character
template<uint8_t Char, conv_func next = unmatched>
inline constexpr uint8_t character(uint8_t c) {
  return 1 + (c == Char ? 0 : next(c));
}

/// Include a range of characters
template<uint8_t from, uint8_t to, conv_func next = unmatched>
inline constexpr uint8_t range(uint8_t c) {
  return 1 + (c >= from && c <= to ? c - from : to - from + next(c));
}

/// Include 0-9
template<conv_func next = unmatched>
inline constexpr uint8_t numeric(uint8_t c) {
  return range<'0', '9', next>(c);
}

/// Include a-z
template<conv_func next = unmatched>
inline constexpr uint8_t lower(uint8_t c) {
  return range<'a', 'z', next>(c);
}

/// Include A-Z
template<conv_func next = unmatched>
inline constexpr uint8_t upper(uint8_t c) {
  return range<'A', 'Z', next>(c);
}

/// Include a-zA-Z
template<conv_func next = unmatched>
inline constexpr uint8_t alpha(uint8_t c) {
  return lower<upper<next>>(c);
}

/// Include a-zA-Z (case insensitive)
template<conv_func next = unmatched>
inline constexpr uint8_t alpha_nocase(uint8_t c) {
  return 1 + (
    c >= 'a' && c <= 'z' ? c - 'a' :
    c >= 'A' && c <= 'Z' ? c - 'A' :
    'z' - 'a' + next(c)
  );
}

/// Like AlnumNoCase, but 0-9 are converted to 'OLZEASGTBQ'"
inline constexpr uint8_t l33t_nocase(uint8_t c) noexcept {
  // Nope. "OLZEASGTBQ"['0' - c] is not constexpr in C++11
  return 1 + (
    c == '0' ? 'O' - 'A' :
    c == '1' ? 'L' - 'A' :
    c == '2' ? 'Z' - 'A' :
    c == '3' ? 'E' - 'A' :
    c == '4' ? 'A' - 'A' :
    c == '5' ? 'S' - 'A' :
    c == '6' ? 'G' - 'A' :
    c == '7' ? 'T' - 'A' :
    c == '8' ? 'B' - 'A' :
    c == '9' ? 'Q' - 'A' :
    c >= 'a' && c <= 'z' ? c - 'a' :
    c >= 'A' && c <= 'Z' ? c - 'A' :
    'z' - 'a' + 1
  );
}

/**
 * Those are optimized for execution time but may include other characters.
 */
namespace fast {

/// Use all characters (0-255)
inline constexpr uint8_t raw(uint8_t c) noexcept {
  return c;
}

/// Use ascii characters (0-127), everything beyound is will be truncated to 127
inline constexpr uint8_t ascii(uint8_t c) noexcept {
  return (c <= 127 ? c : 127);
}

// Use 0-9,.
template<conv_func next = unmatched>
inline constexpr uint8_t floatic(uint8_t c) noexcept {
  return range<',', '9', next>(c);
}

/// Use a-zA-Z_
template<conv_func next = unmatched>
inline constexpr uint8_t alpha(uint8_t c) noexcept {
  return range<'A', 'z', next>(c);
}

/// Use a-zA-Z_ (converted to lower case)
template<conv_func next = unmatched>
inline constexpr uint8_t alpha_nocase(uint8_t c) noexcept {
  return 1 + (c >= 'A' && c <= 'z' ? (c&~0x20) - 'A' : 'Z' - 'A' + next(c));
}

using StringPack::lower;
using StringPack::upper;
using StringPack::numeric;

} // namespace fast

template<conv_func conv>
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

  static inline std::string characters() noexcept
  { return conv_info<conv>::characters(); }
};

using Raw         = packer<fast::raw>;
using ASCII       = packer<fast::ascii>;
using Alpha       = packer<fast::alpha>;
using AlphaNoCase = packer<fast::alpha_nocase>;
using Floatic     = packer<fast::floatic>;
using Lower       = packer<fast::lower>;
using Upper       = packer<fast::upper>;
using Numeric     = packer<fast::numeric>;
using Alnum       = packer<numeric<alpha<character<'_'>>>>;
using AlnumNoCase = packer<numeric<alpha_nocase<character<'_'>>>>;
using L33tNoCase  = packer<l33t_nocase>;

} // namespace StringPack

#endif
