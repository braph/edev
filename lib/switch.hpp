#ifndef LIB_STRINGPACK_HPP
#define LIB_STRINGPACK_HPP

// Stolen and adapted from https://github.com/alipha/cpp/blob/master/switch_pack/switch_pack.h

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
enum:uint64_t { overflow = ~static_cast<uint64_t>(0) };

// ============================================================================
// Helper
// ============================================================================

constexpr int bitlength(unsigned char i) {
  return (
    i < 2   ? 1 :
    i < 4   ? 2 :
    i < 8   ? 3 :
    i < 16  ? 4 :
    i < 32  ? 5 :
    i < 64  ? 6 :
    i < 128 ? 7 : 8);
}

template<conv_func conv>
constexpr uint8_t max_impl(int i) {
  return (i > 255 ? 0 : (conv(i) | max_impl<conv>(i + 1)));
}

template<conv_func conv>
constexpr uint8_t max() {
  return max_impl<conv>(1);
}

template<conv_func conv>
constexpr size_t conv_shift_count() {
  return bitlength(max<conv>());
}

template<conv_func conv>
constexpr size_t conv_max_strlen() {
  return 64 / bitlength(max<conv>());
}

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
  size_t bit_shift  = conv_shift_count<conv>(),
  size_t max_length = conv_max_strlen<conv>()
>
constexpr uint64_t pack_compiletime(const char (&s)[N], size_t i) {
  static_assert(bit_shift == conv_shift_count<conv>(), "Template parameter is not meant to be set by the user!");
  static_assert(max_length == conv_max_strlen<conv>(), "Template parameter is not meant to be set by the user!");
  static_assert(N - 1 <= max_length, "String exceeds maximum length holdable by the integer type");

  return i < N - 1
    ? (static_cast<uint64_t>(conv(s[i])) << (i * bit_shift)) | pack_compiletime<conv>(s, i + 1)
    : 0;
}

/**
 * Runtime pack implementation.
 *
 * Returns the packed chars of `s` as an integer or `overflow` if the
 * input string exceeded the maximum length.
 */
template<
  conv_func conv,
  size_t bit_shift = conv_shift_count<conv>(),
  size_t max_length = conv_max_strlen<conv>()
>
uint64_t pack_runtime(const char* s) noexcept {
  static_assert(bit_shift == conv_shift_count<conv>(), "Template parameter is not meant to be set by the user!");
  static_assert(max_length == conv_max_strlen<conv>(), "Template parameter is not meant to be set by the user!");

  uint64_t result = 0;

  for (size_t i = 0; i < max_length && *s; ++i)
    result |= static_cast<uint64_t>(conv(*s++)) << (i * bit_shift);

  return *s ? overflow : result;
}

template<
  conv_func conv,
  size_t bit_shift = conv_shift_count<conv>(),
  size_t max_length = conv_max_strlen<conv>()
>
uint64_t pack_runtime(const char* s, size_t len) noexcept {
  static_assert(bit_shift == conv_shift_count<conv>(), "Template parameter is not meant to be set by the user!");
  static_assert(max_length == conv_max_strlen<conv>(), "Template parameter is not meant to be set by the user!");

  if (len > max_length)
    return overflow;

  uint64_t result = 0;
  while (len) {
    --len;
    result |= static_cast<uint64_t>(conv(s[len])) << (len * bit_shift);
  }

  return result;
}


// ============================================================================
// Building blocks for creating sets of possible `character sets`
// ============================================================================

/// Include a single character
template< uint8_t Char, conv_func next >
inline constexpr uint8_t character(uint8_t c) {
  return 1 + (c == Char ? 0 : next(c));
}

/// Include a range of characters
template< uint8_t from, uint8_t to, conv_func next >
inline constexpr uint8_t range(uint8_t c) {
  return 1 + (c >= from && c <= to ? c - from : to - from + next(c));
}

/// Include 0-9
template< conv_func next >
inline constexpr uint8_t numeric(uint8_t c) {
  return range<'0', '9', next>(c);
}

/// Include a-z
template< conv_func next >
inline constexpr uint8_t lower(uint8_t c) {
  return range<'a', 'z', next>(c);
}

/// Include A-Z
template< conv_func next >
inline constexpr uint8_t upper(uint8_t c) {
  return range<'A', 'Z', next>(c);
}

/// Include a-zA-Z
template< conv_func next >
inline constexpr uint8_t alpha(uint8_t c) {
  return lower<upper<next>>(c);
}

/// Include a-zA-Z (case insensitive)
template< conv_func next >
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

template< conv_func conv >
struct packer {
  // `const(!) char[]` is the only case where we want *compile time* implementation!
  template<size_t N>
  static inline constexpr uint64_t pack(const char (&s)[N]) noexcept {
    return pack_compiletime<conv, N>(s, 0);
  }

  template<size_t N>
  static inline constexpr uint64_t pack(char (&s)[N]) noexcept {
    static_assert(N&&0, "Please use `pack_runtime` for non-const char");
    return pack_compiletime<conv, N>(s, 0);
  }

#if 0
  static inline constexpr uint64_t pack(const char *s) noexcept {
    static_assert(0, "pack_constexpr only works on `const char[]`");
    return pack_compiletime<conv, N>(s, 0);
  }
#endif

  // pack_runtime =============================================================
  static inline uint64_t pack_runtime(const char* s) noexcept {
    return StringPack::pack_runtime<conv>(s);
  }

  static inline uint64_t pack_runtime(const char* s, size_t len) noexcept {
    return StringPack::pack_runtime<conv>(s, len);
  }

  static inline uint64_t pack_runtime(const std::string& s) noexcept {
    return StringPack::pack_runtime<conv>(s.c_str(), s.size());
  }

  // pack() ===================================================================
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
  static inline constexpr size_t max_size() noexcept {
    return conv_max_strlen<conv>();
  }

  static inline constexpr int bit_shift() noexcept {
    return conv_shift_count<conv>();
  }
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

#ifdef TEST_STRINGPACK
#include <cassert>
#include <iostream>

// Check if all pack algorithms give the same result
#define test(CLASS, C_STRING) do { \
  assert(CLASS::pack_runtime(C_STRING) == CLASS::pack(C_STRING)); \
  assert(CLASS::pack_runtime(std::string(C_STRING)) == CLASS::pack(C_STRING)); \
} while(0)

int main() {
  {
    using C = StringPack::Numeric;
    std::cout << "Numeric: " << C::max_size() << ':' << C::bit_shift() << std::endl;

    const char* s = "123456789";
    switch (C::pack_runtime(s)) {
      case C("123456789"):
        std::cout << "Success!\n";
    }
  }

  {
    using SC = StringPack::AlphaNoCase;
    switch (SC::pack_runtime("progressBar")) {
      case SC::pack("progressbar"):
        std::cout << "Success!\n";
    }
  }

  {
    using SC = StringPack::AlnumNoCase;
    assert(SC::pack("A") == SC::pack("a"));

    switch (SC::pack("A")) {
      case SC::pack("a"):
        std::cout << "LOL" << std::endl;
    }
  }

  {
    using SC = StringPack::Generic;

    assert(0   == SC::pack(""));
    assert('a' == SC::pack("a"));
    assert(SC::pack_runtime("overflow0")   == StringPack::overflow);
  //assert(pack("overflow0")   == pack("overflow0"));
  //assert(pack("overflow12")  == pack("overflow12"));

    test(SC, "");
    test(SC, "a");
    test(SC, "aa");
    test(SC, "aaa");
    test(SC, "aaaa");
    test(SC, "aaaaa");
    test(SC, "aaaaaa");
    test(SC, "aaaaaaa");
    test(SC, "aaaaaaaa");
    test(SC, "ab");
    test(SC, "abc");
    test(SC, "abcd");
    test(SC, "abcde");
    test(SC, "abcdef");
    test(SC, "abcdefg");
    test(SC, "abcdefgh");
    test(SC, "b");
    test(SC, "x");
  }

  {
    using SC = StringPack::Lower;

    assert(0 == SC::pack(""));
    assert(1 == SC::pack("a"));
    assert(26 == SC::pack("z"));
    assert(27 == SC::pack("_"));
    assert(28 == SC::pack("+"));

    assert(SC::pack_runtime("overflow_overflow")   == StringPack::overflow);
  //assert(pack("overflow0")   == pack("overflow0"));
  //assert(pack("overflow12")  == pack("overflow12"));

    test(SC, "");

    test(SC, "a");
    test(SC, "aa");
    test(SC, "aaa");
    test(SC, "aaaa");
    test(SC, "aaaaa");
    test(SC, "aaaaaa");
    test(SC, "aaaaaaa");
    test(SC, "aaaaaaaa");

    test(SC, "ab");
    test(SC, "abc");
    test(SC, "abcd");
    test(SC, "abcde");
    test(SC, "abcdef");
    test(SC, "abcdefg");
    test(SC, "abcdefgh");
    test(SC, "b");
    test(SC, "x");
  }
}
#endif
