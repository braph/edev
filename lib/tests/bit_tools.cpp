#include "../bit_tools.hpp"

#include <cassert>
#include <cstdint>

int main() {
  // extract_bits


  // replace_bits
  assert(0xFF == replace_bits<uint8_t>(0x00, 0xFF, 0, 8) );
  assert(0xF0 == replace_bits<uint8_t>(0x00, 0xFF, 4, 4) );
  assert(0x0F == replace_bits<uint8_t>(0x00, 0xFF, 0, 4) );

  assert(0x00 == replace_bits<uint8_t>(0xFF, 0x00, 0, 8) );
}
