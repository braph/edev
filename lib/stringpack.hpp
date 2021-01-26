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
#include <limits>

namespace StringPack {

using conv_func = unsigned(*)(unsigned);
enum : uint64_t { overflow  = ~uint64_t(0) };
enum : unsigned { skip_char = 0xFF00       };

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
  static constexpr unsigned max_bitlength() {
    return bitlength(bitmask() & 0xFF /* remove skip_char */);
  }

  static constexpr unsigned max_strlen() {
    return 64 / max_bitlength();
  }

  static constexpr unsigned char_count(unsigned c = 0, unsigned return_ = 0) {
    return c > 255 ? return_ : char_count(c + 1,
      (conv(c) != skip_char && conv(c) > return_) ? conv(c) : return_);
  }

  static constexpr bool has_skip_char(unsigned c = 0) {
    return c > 255 ? false : ((conv(c) == skip_char ? true : has_skip_char(c + 1)));
  }

  static std::string characters() {
    unsigned sentinel_ = sentinel();
    std::string r;
    for (unsigned c = 0; c < 255; ++c)
      if (conv(c) != sentinel_)
        r.push_back(c);
    return r;
  }

private:
  static constexpr unsigned bitmask(unsigned i = 0) {
    return (i > 255 ? 0 : (conv(i) | bitmask(i + 1)));
  }

  static constexpr unsigned sentinel(unsigned c = 0, unsigned return_ = 0) {
    return c > 255 ? return_ : sentinel(c + 1,
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
  size_t bit_shift     = conv_info<conv>::max_bitlength(),
  size_t max_length    = conv_info<conv>::max_strlen(),
  bool   has_skip_char = conv_info<conv>::has_skip_char()
>
constexpr uint64_t pack_compiletime(const char (&s)[N], size_t i, size_t shift = 0, uint64_t return_ = 0) {
  static_assert(bit_shift  == conv_info<conv>::max_bitlength(), "DO NOT SET THIS TEMPLATE PARAMETER");
  static_assert(max_length == conv_info<conv>::max_strlen(),    "DO NET SET THIS TEMPLATE PARAMETER");
  static_assert(N - 1 <= max_length, "String exceeds maximum length holdable by the integer type");

  return i >= N - 1 ? return_ : (
    (has_skip_char && conv(unsigned(s[i])) == skip_char) ? (
      pack_compiletime<conv>(s, i + 1, shift, return_)
    ) : (
      pack_compiletime<conv>(s, i + 1, shift + 1, return_ |
        uint64_t(conv(unsigned(s[i]))) << (shift * bit_shift))
    )
  );
}

/**
 * Runtime pack implementation.
 *
 * Returns the packed chars of `s` as an integer or `overflow` if the
 * input string exceeded the maximum length.
 */
template<conv_func conv>
uint64_t pack_runtime(const unsigned char* s) noexcept {
  enum : size_t {
    bit_shift     = conv_info<conv>::max_bitlength(),
    max_length    = conv_info<conv>::max_strlen(),
    has_skip_char = conv_info<conv>::has_skip_char()
  };

  uint64_t result = 0;

  if (has_skip_char) {
    size_t shift = 0;
    for (; *s; ++s) {
      const uint64_t converted = conv(*s);
      if (converted != skip_char)
        result |= converted << (shift++ * bit_shift);
    }
    return shift > max_length ? overflow : result;
  }
  else
  {
    size_t i;
    for (i = 0; s[i]; ++i)
      result |= uint64_t(conv(s[i])) << (i * bit_shift);
    return i > max_length ? overflow : result;
  }
}

template<conv_func conv>
uint64_t pack_runtime(const unsigned char* s, size_t len) noexcept {
  enum : size_t {
    bit_shift     = conv_info<conv>::max_bitlength(),
    max_length    = conv_info<conv>::max_strlen(),
    has_skip_char = conv_info<conv>::has_skip_char()
  };

  uint64_t result = 0;

  if (has_skip_char) {
    size_t shift = 0;
    for (size_t i = 0; i < len; ++i) {
      const uint64_t converted = conv(s[i]);
      if (converted != skip_char)
        result |= converted << (shift++ * bit_shift);
    }

    return shift > max_length ? overflow : result;
  }
  else
  {
    if (len > max_length)
      return overflow;

    size_t shift = bit_shift * len;
    while (len)
      result |= uint64_t(conv(s[--len])) << (shift -= bit_shift);

    return result;
  }
}

// ============================================================================
// Building blocks for creating sets of possible `character sets`
// ============================================================================

template<unsigned base>
inline constexpr unsigned combine(unsigned) {
  return base; // Unhandeled TODO
}

template<unsigned base, conv_func f, conv_func ... T>
inline constexpr unsigned combine(unsigned c) {
  return f(c) ? base + f(c) : combine<base + conv_info<f>::char_count(), T ...>(c);
}

template<conv_func ... Fs>
inline constexpr unsigned combine(unsigned c) {
  return combine<0, Fs ...>(c);
}


///////////////////////////////////////////////////////////////////////////////
struct bitset_t {
  uint64_t d[4];
  bool constexpr operator[](unsigned i) const { return d[i/64] & 1ULL << (i%64); }
};

template<uint64_t a, uint64_t b, uint64_t c, uint64_t d, unsigned ... Chars>
struct make_bitset_t;

template<uint64_t a, uint64_t b, uint64_t c, uint64_t d, unsigned Char, unsigned ... Chars>
struct make_bitset_t<a, b, c, d, Char, Chars...> {
  static constexpr bitset_t value() {
    return make_bitset_t<
      a | ((Char / 64 == 0) ? (1ULL << (Char % 64)) : 0),
      b | ((Char / 64 == 1) ? (1ULL << (Char % 64)) : 0),
      c | ((Char / 64 == 2) ? (1ULL << (Char % 64)) : 0),
      d | ((Char / 64 == 3) ? (1ULL << (Char % 64)) : 0),
    Chars...>::value();
  }
};

template<uint64_t a, uint64_t b, uint64_t c, uint64_t d>
struct make_bitset_t<a, b, c, d> {
  static constexpr bitset_t value() { return bitset_t{a,b,c,d}; };
};

template<unsigned ... Chars>
inline constexpr unsigned skip_chars(unsigned c) {
  return make_bitset_t<0, 0, 0, 0, Chars...>::value()[c] * skip_char;
}
///////////////////////////////////////////////////////////////////////////////


/// Include a single character
template<unsigned Char>
inline constexpr unsigned character(unsigned c) {
  return c == Char;
}

/// Include a range of characters
template<unsigned from, unsigned to>
inline constexpr unsigned range(unsigned c) {
  return (c >= from && c <= to ? c - from + 1 : 0);
}

/// Include 0-9
inline constexpr unsigned numeric(unsigned c) {
  return range<'0', '9'>(c);
}

/// Include a-z
inline constexpr unsigned lower(unsigned c) {
  return range<'a', 'z'>(c);
}

/// Include A-Z
inline constexpr unsigned upper(unsigned c) {
  return range<'A', 'Z'>(c);
}

/// Include a-zA-Z
inline constexpr unsigned alpha(unsigned c) {
  return combine<lower, upper>(c);
}

/// Include a-zA-Z (case insensitive)
inline constexpr unsigned alpha_nocase(unsigned c) {
  return (
    c >= 'a' && c <= 'z' ? c - 'a' + 1:
    c >= 'A' && c <= 'Z' ? c - 'A' + 1:
    0);
}

/// Like AlnumNoCase, but 0-9 are converted to 'OLZEASGTBQ'
inline constexpr unsigned l33t_nocase(unsigned c) noexcept {
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

inline constexpr unsigned unmatched(unsigned) noexcept {
  return 1;
}

/// Use all characters (0-255)
inline constexpr unsigned raw(unsigned c) noexcept {
  return c;
}

/// Use ascii characters (0-127), everything beyound is will be truncated to 127
inline constexpr unsigned ascii(unsigned c) noexcept {
  return (c <= 127 ? c : 127);
}

/**
 * These functions guarantee the best execution time.
 * Howewer, they may include some other characters.
 */
namespace fast {

inline constexpr unsigned upper(unsigned c) noexcept { return c & ~0x20U; }

// Use 0-9,.
inline constexpr unsigned floatic(unsigned c) noexcept {
  return range<',', '9'>(c);
}

/// Use a-zA-Z_
inline constexpr unsigned alpha(unsigned c) noexcept {
  return range<'A', 'z'>(c);
}

/// Use a-zA-Z_ (converted to upper case)
inline constexpr unsigned alpha_nocase(unsigned c) noexcept {
  return (c >= 'A' && c <= 'z' ? upper(c) - 'A' + 1 : 0);
}

} // namespace fast


template<conv_func conv>
struct packer {
  // packer::pack() --- compile time ==========================================
  template<size_t N>
  static inline constexpr uint64_t pack(const char (&s)[N]) noexcept
  { return pack_compiletime<conv, N>(s, 0); }

  // packer::pack() --- run time ==============================================
  template<typename ... T>
  static inline uint64_t pack(const T& ... args) noexcept
  { return packer::pack_runtime(args...); }

  // packer::packer() --- compile time ========================================
  template<size_t N>
  inline constexpr packer(const char (&s)[N]) noexcept
    : _s(pack_compiletime<conv, N>(s, 0))
  {}

  // packer::packer() --- run time ============================================
  template<typename ... T>
  inline packer(const T& ... args) noexcept
    : _s(packer::pack_runtime(args...))
  {}


  static inline uint64_t pack_runtime(const char* s, size_t len) noexcept
  { return StringPack::pack_runtime<conv>(reinterpret_cast<const unsigned char*>(s), len); }

  static inline uint64_t pack_runtime(const unsigned char* s, size_t len) noexcept
  { return StringPack::pack_runtime<conv>(reinterpret_cast<const unsigned char*>(s), len); }

  static inline uint64_t pack_runtime(const char* s) noexcept
  { return StringPack::pack_runtime<conv>(reinterpret_cast<const unsigned char*>(s)); }

  static inline uint64_t pack_runtime(const unsigned char* s) noexcept
  { return StringPack::pack_runtime<conv>(s); }

  static inline uint64_t pack_runtime(const std::string& s) noexcept
  { return StringPack::pack_runtime<conv>(s.c_str(), s.size()); }


  // Information ==============================================================
  static inline constexpr unsigned max_strlen() noexcept
  { return conv_info<conv>::max_strlen(); }

  static inline constexpr unsigned bit_shift() noexcept
  { return conv_info<conv>::max_bitlength(); }

  static inline std::string characters() noexcept
  { return conv_info<conv>::characters(); }

  static inline constexpr unsigned char_count() noexcept
  { return conv_info<conv>::char_count(); }


  constexpr inline operator uint64_t() const noexcept { return _s; }
private:
  uint64_t _s;
};

using Raw         = packer<raw>;
using ASCII       = packer<ascii>;
using Alpha       = packer<combine<fast::alpha>>;
using AlphaNoCase = packer<combine<fast::alpha_nocase>>;
using Floatic     = packer<combine<fast::floatic>>;
using Lower       = packer<combine<lower, unmatched>>;
using Upper       = packer<combine<upper>>;
using Numeric     = packer<combine<numeric>>;
using Alnum       = packer<combine<numeric, fast::alpha>>;
using AlnumNoCase = packer<combine<numeric, fast::alpha_nocase>>;
using L33tNoCase  = packer<combine<l33t_nocase>>;

} // namespace StringPack

#endif
