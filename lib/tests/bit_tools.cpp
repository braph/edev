#include "../bit_tools.hpp"

#include <cassert>
#include <cstdint>

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
}
