#include "../bit_tools.hpp"

#include <climits>
#include <cassert>
#include <cstdint>
#include <vector>
#include <iostream>
#include <initializer_list>

static bool test_iterate_set_bits(unsigned N, const std::initializer_list<int>& l) {
  auto x {iterate_set_bits(N)};
//for (auto i : x) std::cout << i << ','; std::cout << '\n';
  return std::vector<int>(x.begin(), x.end()) == std::vector<int>(l);
}
#define test_iterate_set_bits(...) assert((test_iterate_set_bits)(__VA_ARGS__))

int main() {
  // bitlength_const
  assert(0  == bitlength_const(0));
  assert(1  == bitlength_const(1));
  assert(2  == bitlength_const(2));
  assert(2  == bitlength_const(3));
  assert(3  == bitlength_const(4));
  assert(64 == bitlength_const(ULLONG_MAX));

  // extract_bits
  assert(0xFF == extract_bits<uint16_t>(0x00FF, 0, 8));
  assert(0xFF == extract_bits<uint16_t>(0x0FF0, 4, 8));
  assert(0xFF == extract_bits<uint16_t>(0xFF00, 8, 8));

  // replace_bits
  assert(0xFF00 == replace_bits<uint16_t>(0x0000, 0xFF, 8, 8));
  assert(0x0FF0 == replace_bits<uint16_t>(0x0000, 0xFF, 4, 8));
  assert(0x00FF == replace_bits<uint16_t>(0x0000, 0xFF, 0, 8));

  assert(0xFF11 == replace_bits<uint16_t>(0x1111, 0xFF, 8, 8));
  assert(0x1FF1 == replace_bits<uint16_t>(0x1111, 0xFF, 4, 8));
  assert(0x11FF == replace_bits<uint16_t>(0x1111, 0xFF, 0, 8));

  // iterate_bits
  test_iterate_set_bits(1, {0});
  test_iterate_set_bits(2, {1});
  test_iterate_set_bits(0xFFFFFFFF, {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31});
}
