#ifndef LIB_PACKED_TRAITS
#define LIB_PACKED_TRAITS

#include "../bit_tools.hpp"

#include <type_traits>

#if 1
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
    auto dataIndex = (index * bits) / TBits;
    auto bitOffset = (index * bits) % TBits;

    if (bitOffset + bits <= TBits) {
      Unsigned e = data[dataIndex];
      return (e >> bitOffset) &~(OxFFFF << bits);
    }
    else {
      Doubled e = (Doubled(data[dataIndex + 1]) << TBits) | data[dataIndex + 0];
      return (e >> bitOffset) &~(OxFFFFFFFF << bits);
    }
  }

  static inline void set(T* data, int bits, int index, T value) noexcept {
    auto dataIndex = (index * bits) / TBits;
    auto bitOffset = (index * bits) % TBits;

    if (bitOffset + bits <= TBits) {
      Unsigned e = data[dataIndex];
      data[dataIndex] = replace_bits<Unsigned>(e, value, bitOffset, bits);
    } else {
      Doubled e = (Doubled(data[dataIndex+1]) << TBits) | data[dataIndex+0];
      e = replace_bits<Doubled>(e, value, bitOffset, bits);
      data[dataIndex+1] = e >> TBits;
      data[dataIndex+0] = e & OxFFFF;
    }
  }
};
#else /* Backup */
#define LIB_PACKED_TRAITS__USE_POSSIBLE_FASTER_IMPLEMENTATION 0
template<class store_type>
struct packed_traits {

  union BitShiftHelper {
    struct {
      uint32_t u1;
      uint32_t u2;
    } u32;
    uint64_t u64;
  };

  static inline store_type get(store_type* _data, int _bits, int index) noexcept {
    unsigned int dataIndex = (index * _bits) / int(BITSOF(store_type));
    unsigned int bitOffset = (index * _bits) % int(BITSOF(store_type));

#if ! LIB_PACKED_TRAITS__USE_POSSIBLE_FASTER_IMPLEMENTATION
    if (bitOffset + _bits <= 32) {
      store_type e = _data[dataIndex];
      e = (e >> bitOffset) &~(0xFFFFFFFFu << _bits);
      return e;
    }
    else
#endif
    {
      BitShiftHelper store;
      store.u32.u1 = _data[dataIndex];
      store.u32.u2 = _data[dataIndex+1];
      store_type e = (store.u64 >> bitOffset) &~(0xFFFFFFFFu << _bits);
      return e;
    }
  }

  static inline void set(store_type* _data, int _bits, int index, store_type value) noexcept {
    unsigned int dataIndex = (index * _bits) / int(BITSOF(store_type));
    unsigned int bitOffset = (index * _bits) % int(BITSOF(store_type));

    if (bitOffset + _bits <= sizeof(store_type) * CHAR_BIT) {
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
};
#endif

#endif
