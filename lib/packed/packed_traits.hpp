#ifndef LIB_PACKED_TRAITS
#define LIB_PACKED_TRAITS

// This has to be benchmarked again?
#define LIB_PACKED_TRAITS__USE_POSSIBLE_FASTER_IMPLEMENTATION 0

template<class store_type, class value_type>
struct packed_traits {

  union BitShiftHelper {
    struct {
      uint32_t u1;
      uint32_t u2;
    } u32;
    uint64_t u64;
  };

  static inline value_type get(store_type* _data, size_t _bits, size_t index) noexcept {
    unsigned int dataIndex = (index * _bits) / 32;
    unsigned int bitOffset = (index * _bits) % 32;

#if ! LIB_PACKED_TRAITS__USE_POSSIBLE_FASTER_IMPLEMENTATION
    if (bitOffset + _bits <= 32) {
      uint32_t e = _data[dataIndex];
      e = (e >> bitOffset) &~(0xFFFFFFFFu << _bits);
      return value_type(e);
    }
    else
#endif
    {
      BitShiftHelper store;
      store.u32.u1 = _data[dataIndex];
      store.u32.u2 = _data[dataIndex+1];
      uint32_t e = (store.u64 >> bitOffset) &~(0xFFFFFFFFu << _bits);
      return value_type(e);
    }
  }

  static inline void set(store_type* _data, size_t _bits, size_t index, value_type value) noexcept {
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

};

#endif
