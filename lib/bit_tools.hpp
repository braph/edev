#ifndef BIT_TOOLS_HPP
#define BIT_TOOLS_HPP

#include <limits>
#include <climits>
#include <cstddef>
#include <type_traits>
#include <iterator>
#include <string.h> // ffs

#define BITSOF(T) (CHAR_BIT * sizeof(T))

template<class T, bool BitState = 1>
struct BitIterator {
  using value_type = unsigned;
  using reference = void; 
  using pointer = void;
  using iterator = BitIterator;
  using iterator_category = std::input_iterator_tag;
  using difference_type = int;
  T val;
  unsigned i;
  inline BitIterator(T value, unsigned ii = 0) noexcept : val(value), i(ii) { seek(); }
  inline iterator  begin()                   const noexcept { return {val, 0};           }
  inline iterator  end()                     const noexcept { return {0, BITSOF(T)};     }
  inline iterator& operator++()                    noexcept { ++i; seek(); return *this; }
  inline unsigned  operator*()               const noexcept { return i;                  }
  inline bool operator==(const iterator& it) const noexcept { return i == it.i;          }
  inline bool operator!=(const iterator& it) const noexcept { return i != it.i;          }
private:
  inline void  seek() noexcept { while ((!!(val & (1<<i))) != BitState && i < BITSOF(T)) ++i; }
};

template<class T>
struct BitIterator<T, 1> {
  using value_type = unsigned;
  using reference = void; 
  using pointer = void;
  using iterator = BitIterator;
  using iterator_category = std::input_iterator_tag;
  using difference_type = int;
  unsigned i;
  T val;
  inline BitIterator(T value = 0, unsigned ii = 0) noexcept : val(value), i(ii)          {}
  inline iterator  begin()                   const noexcept { int i = ffs(val); return {val>>i, i}; }
  inline iterator  end()                     const noexcept { return {};        }
  inline unsigned  operator*()               const noexcept { return i - 1;     }
  inline bool operator==(const iterator& it) const noexcept { return i == it.i; }
  inline bool operator!=(const iterator& it) const noexcept { return i != it.i; }
  inline iterator& operator++()                    noexcept {
    auto old = i;
    i = ffs(val);
    if (i) {
      val >>= i;
      i += old;
    }
    return *this;
  }
};

template<class T>
BitIterator<T> iterate_set_bits(T val) {
  return BitIterator<T, 1>(val);
}

template<class T>
constexpr int bitlength_const(T n) noexcept {
  return (n ? 1 + bitlength_const(n >> 1) : 0);
}

#if (defined(__GNUC__) || defined(__clang__))
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
#else
template<class T> int bitlength(T n) noexcept { return bitlength_const(n); }
#endif

// ============================================================================

/**
 * Example for replace_bits<uint_16t>(18, 7, 3, 3)
 *
 * BIT_COUNT: 16
 * 0xFFFF:    11111111 11111111 -- All bits set
 * src:       00000000 00010010 -- Initial value that is going to be altered
 * val:       00000000 00000111 -- Value to to insert
 * offset:                 ^---------- where to put the value
 * len:                  ^------------ length of the value
 * mask:      00000000 11100000 <--- gives mask
 *
 * clearing `src` with `& ~mask`:
 *            00000000 00010010
 *            00000000 00000010
 *
 * XXX this example is wrong :D
 */
template<typename T>
inline T replace_bits(T src, T val, int offset, int len) {
  using Unsigned_T = typename std::make_unsigned<T>::type;
  enum: Unsigned_T { OxFFFF = std::numeric_limits<Unsigned_T>::max() };
  const Unsigned_T u_src = static_cast<Unsigned_T>(src);
  const Unsigned_T u_val = static_cast<Unsigned_T>(val) & ~(OxFFFF << len); // trim `val` to `len` bits
  const Unsigned_T mask = (~(OxFFFF << len)) << offset;
  return static_cast<T>((u_src & ~mask) | (u_val << offset));
}

/**
 * extract_bits(00001110, 1, 3) -> 00000111
 */
template<typename T>
inline T extract_bits(T src, int offset, int len) {
  using Unsigned_T = typename std::make_unsigned<T>::type;
  enum: Unsigned_T { OxFFFF = std::numeric_limits<Unsigned_T>::max() };
  const Unsigned_T unsigned_src = static_cast<Unsigned_T>(src);
  return static_cast<T>((unsigned_src >> offset) & ~(OxFFFF << len));
}

#endif
