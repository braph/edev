#ifndef LIB_PACKED_TRAITS
#define LIB_PACKED_TRAITS

#include "../bit_tools.hpp"

#include <type_traits>

template<class T>
struct packed_traits {
  using Unsigned = typename std::make_unsigned<T>::type;
  using Doubled =
    typename std::conditional<sizeof(T) == 1, uint16_t, 
    typename std::conditional<sizeof(T) == 2, uint32_t,
    typename std::conditional<sizeof(T) == 4, uint64_t, void>::type>::type>::type;

  static_assert(!std::is_void<Doubled>::value, "packed_traits<int64_t> not yet implented!");

  enum { TBits      = BITSOF(T) };
  enum { OxFFFF     = std::numeric_limits<Unsigned>::max() };
  enum { OxFFFFFFFF = std::numeric_limits<Doubled>::max()  };

  static inline T get(T* data, int bits, int index) noexcept {
    const auto dataIndex = (index * bits) / TBits;
    const auto bitOffset = (index * bits) % TBits;

    Doubled e = data[dataIndex];
    if (bitOffset + bits > TBits)
      e |= Doubled(data[dataIndex + 1]) << TBits;
    return extract_bits(e, bitOffset, bits);
  }

  static inline void set(T* data, int bits, int index, T value) noexcept {
    const auto dataIndex = (index * bits) / TBits;
    const auto bitOffset = (index * bits) % TBits;

    Doubled e = data[dataIndex];
    if (bitOffset + bits > TBits)
      e |= Doubled(data[dataIndex + 1]) << TBits;

    e = replace_bits<Doubled>(e, value, bitOffset, bits);

    data[dataIndex] = e & OxFFFF;
    if (bitOffset + bits > TBits)
      data[dataIndex+1] = e >> TBits;
  }
};

#if 0
namespace compiletime {
template<class T, uint64_t Result, unsigned Bits, T ... Values>
struct pack;

template<class T, uint64_t Result, unsigned Bits, T Value, T ... Values>
struct pack<T, Result, Bits, Value, Values...> {
  static constexpr uint64_t value() {
    return pack<T, Result << Bits | Value, Bits, Values...>::value();
  }
};

template<class T, uint64_t Result, unsigned Bits>
struct pack<T, Result, Bits> {
  static constexpr uint64_t value() { return Result; }
};
}
#endif

#endif
