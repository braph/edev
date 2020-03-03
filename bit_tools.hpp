#ifndef _BIT_TOOLS_HPP
#define _BIT_TOOLS_HPP

// Unsigned ===================================================================
inline int bitlength(unsigned long long n) {
  return (!n ? 0 :
    static_cast<int>(sizeof(long long) * CHAR_BIT) - __builtin_clzll(n));
}

inline int bitlength(unsigned long n) {
  return (!n ? 0 :
    static_cast<int>(sizeof(long) * CHAR_BIT) - __builtin_clzl(n));
}

inline int bitlength(unsigned int n) {
  return (!n ? 0 :
    static_cast<int>(sizeof(int) * CHAR_BIT) - __builtin_clz(n));
}

inline int bitlength(unsigned short n) {
  return (!n ? 0 :
    static_cast<int>(sizeof(int) * CHAR_BIT) - __builtin_clz(n));
}

inline int bitlength(unsigned char n) {
  return (!n ? 0 :
    static_cast<int>(sizeof(int) * CHAR_BIT) - __builtin_clz(n));
}

// Signed =====================================================================
inline int bitlength(long long n) {
  return bitlength(static_cast<unsigned long long>(n));
}

inline int bitlength(long n) {
  return bitlength(static_cast<unsigned long>(n));
}

inline int bitlength(int n) {
  return bitlength(static_cast<unsigned int>(n));
}

inline int bitlength(short n) {
  return bitlength(static_cast<unsigned short>(n));
}

inline int bitlength(char n) {
  return bitlength(static_cast<unsigned char>(n));
}

/* Example for replace_bits<uint_16t>(18, 7, 3, 3)
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

template<typename TUIntType>
inline TUIntType replace_bits(TUIntType src, TUIntType val, int offset, int len) {
  enum { BIT_COUNT = CHAR_BIT * sizeof(TUIntType) };
  const TUIntType OxFFFF = std::numeric_limits<TUIntType>::max();

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

#include<vector>
#include<iostream>
#include<bitset>
// 0100|0000 0010|0000 
template<typename TUIntType, typename TIterator>
TUIntType compress(TIterator _it, TIterator _end, int bitwidth) {
  if (_it == _end)
    return 0;
  DynamicArray<TUIntType, 8> sorted(_it, _end);

  auto it = sorted.begin();
  auto end = sorted.end();
  std::sort(it, end, std::greater<TUIntType>());

//std::cout << "First value: " << *it << std::endl;
  TUIntType result = replace_bits<TUIntType>(0, *it, 0, bitwidth);
//std::cout << "Result: " << std::bitset<32>(result) << std::endl;

  int offset = bitwidth;

  for (++it; it != end; ++it) {
//  std::cout << "Inserting " << *it << " on " << offset << " bitwidth: " << bitwidth << std::endl;

    result= replace_bits<TUIntType>(result,*it, offset, bitwidth);
    offset += bitwidth;
    bitwidth = bitlength(*it);

//  std::cout << "Result: " << std::bitset<32>(result) << std::endl;
  }
  return result;
}

template<typename TUIntType>
struct BitUnpacker {
  TUIntType packed;
  int bitwidth;
  int offset;

  BitUnpacker(TUIntType packed, int bitwidth)
  : packed(packed), bitwidth(bitwidth), offset(0) {}

  TUIntType next() {
    TUIntType value;

    if (! offset) {
      // First integer, special case:
      // Bitwidth stays! It is NOT obtained by bitlength(value)
      value = extract_bits<TUIntType>(packed, offset, bitwidth);
      offset = bitwidth;
    }
    else {
      value = extract_bits<TUIntType>(packed, offset, bitwidth);
      offset += bitwidth;
      bitwidth = bitlength(value);
    }
    return value;
  }
};

template<typename TUIntType, typename TContainer>
void uncompress(TContainer& container, TUIntType packed, int bitwidth) {
  container.clear();
  TUIntType value = extract_bits<TUIntType>(packed, 0, bitwidth);
  container.push_back(value);
//std::cout << "First value: " << value << " (" << std::endl;
  int offset = bitwidth;

  while ((value = extract_bits<TUIntType>(packed, offset, bitwidth)) > 0) {
//  std::cout << "push back: " << value << std::endl;
    container.push_back(value);
    offset += bitwidth;
    bitwidth = bitlength(value);
  }

#if 0
  const TUIntType OxFFFF = ~static_cast<TUIntType>(0);
  TUIntType value = packed &~(OxFFFF << bitwidth);
  bitwidth = 

  return (src >> offset) &~(OxFFFF << len);
#endif
}

#endif
