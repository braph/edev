#include<iostream>
#include<bitset>
#include<algorithm>
#include "staticvector.hpp"

// 0100|0000 0010|0000 
template<typename TUIntType, typename TIterator>
TUIntType compress(TIterator _it, TIterator _end, int bitwidth) {
  if (_it == _end)
    return 0;
  StaticVector<TUIntType, 8> sorted(_it, _end);

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

