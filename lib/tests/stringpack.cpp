#include "../stringpack.hpp"

#include <cassert>
#include <cstdio>

// Check if all pack algorithms give the same result
#define test(CLASS, C_STRING) do { \
  assert(CLASS::pack_runtime(C_STRING)              == CLASS::pack(C_STRING)); \
  assert(CLASS::pack_runtime(std::string(C_STRING)) == CLASS::pack(C_STRING)); \
} while(0)

#define doc(CLASS, DOC) \
  printf("%-23s | %-5zu | %s\n", #CLASS, CLASS::max_size(), DOC)

int main() {
  printf("%-23s | %-5s | %s\n", "class", "len", "comment");
  doc(StringPack::Generic,     "Fastest (no conversion applied)");
  doc(StringPack::ASCII,       "Fast (chars >= 128 will be converted to 127)");
  doc(StringPack::Alnum,       "");
  doc(StringPack::AlnumNoCase, "");
  doc(StringPack::Alpha,       "");
  doc(StringPack::AlphaNoCase, "");
  doc(StringPack::Upper,       "");
  doc(StringPack::Lower,       "");
  doc(StringPack::L33tNoCase,  "Like AlnumNoCase, but 0-9 are converted to 'OLZEASGTBQ'");
  doc(StringPack::Numeric,     "");

  {
    using SC = StringPack::L33tNoCase;
    switch (SC::pack_runtime("0123456789")) {
      case SC("OLZEASGTBQ"): break;
      default:               throw;
    }
  }

  {
    using SC = StringPack::Numeric;

    const char* s = "123456789";
    switch (SC::pack_runtime(s)) {
      case SC("123456789"): break;
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
      case SC::pack("a"): break;
      default:            throw;
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
    using SC = StringPack::ASCII;
    assert(0   == SC::pack(""));
    assert(127 == SC::pack("\x7F"));
    assert(127 == SC::pack("\x80"));

    test(SC, "nine char");
    assert(SC::pack_runtime("overflow10") == StringPack::overflow);
  }

  {
    using SC = StringPack::Lower;

    assert(0  == SC::pack(""));
    assert(1  == SC::pack("a"));
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
