#include "../stringpack.hpp"

#include <cassert>
#include <iostream>

// Check if all pack algorithms give the same result
#define test(CLASS, C_STRING) do { \
  assert(CLASS::pack_runtime(C_STRING)              == CLASS::pack(C_STRING)); \
  assert(CLASS::pack_runtime(std::string(C_STRING)) == CLASS::pack(C_STRING)); \
} while(0)

int main() {
  {
    using C = StringPack::Numeric;
    std::cout << "Numeric: " << C::max_size() << ':' << C::bit_shift() << std::endl;

    const char* s = "123456789";
    switch (C::pack_runtime(s)) {
      case C("123456789"): break;
      default:             throw;
    }
  }

  {
    using SC = StringPack::AlphaNoCase;
    switch (SC::pack_runtime("progressBar")) {
      case SC::pack("progressbar"): break;
      default:                      throw;
    }
  }

  {
    using SC = StringPack::AlnumNoCase;
    assert(SC::pack("A") == SC::pack("a"));

    switch (SC::pack("A")) {
      case SC::pack("a"):
        std::cout << "LOL" << std::endl;
    }
  }

  {
    using SC = StringPack::Generic;

    assert(0   == SC::pack(""));
    assert('a' == SC::pack("a"));
    assert(SC::pack_runtime("overflow0")   == StringPack::overflow);
  //assert(pack("overflow0")   == pack("overflow0"));
  //assert(pack("overflow12")  == pack("overflow12"));

    test(SC, "");
    test(SC, "a");
    test(SC, "aa");
    test(SC, "aaa");
    test(SC, "aaaa");
    test(SC, "aaaaa");
    test(SC, "aaaaaa");
    test(SC, "aaaaaaa");
    test(SC, "aaaaaaaa");
    test(SC, "ab");
    test(SC, "abc");
    test(SC, "abcd");
    test(SC, "abcde");
    test(SC, "abcdef");
    test(SC, "abcdefg");
    test(SC, "abcdefgh");
    test(SC, "b");
    test(SC, "x");
  }

  {
    using SC = StringPack::Lower;

    assert(0 == SC::pack(""));
    assert(1 == SC::pack("a"));
    assert(26 == SC::pack("z"));
    assert(27 == SC::pack("_"));
    assert(28 == SC::pack("+"));

    assert(SC::pack_runtime("overflow_overflow")   == StringPack::overflow);
  //assert(pack("overflow0")   == pack("overflow0"));
  //assert(pack("overflow12")  == pack("overflow12"));

    test(SC, "");

    test(SC, "a");
    test(SC, "aa");
    test(SC, "aaa");
    test(SC, "aaaa");
    test(SC, "aaaaa");
    test(SC, "aaaaaa");
    test(SC, "aaaaaaa");
    test(SC, "aaaaaaaa");

    test(SC, "ab");
    test(SC, "abc");
    test(SC, "abcd");
    test(SC, "abcde");
    test(SC, "abcdef");
    test(SC, "abcdefg");
    test(SC, "abcdefgh");
    test(SC, "b");
    test(SC, "x");
  }
}
