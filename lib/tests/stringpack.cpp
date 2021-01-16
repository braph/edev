#include "../stringpack.hpp"

#include <cassert>
#include <cstdio>

// Check if all pack algorithms give the same result
#define test(CLASS, C_STRING) do { \
  assert(CLASS::pack_runtime(C_STRING)              == CLASS::pack(C_STRING)); \
  assert(CLASS::pack_runtime(std::string(C_STRING)) == CLASS::pack(C_STRING)); \
} while(0)

#define test_conversion(STRING_1, STRING_2) \
  assert(SC::pack_runtime(STRING_1) == SC::pack(STRING_2))

#define doc(CLASS, DOC) \
  printf("%-23s | %-5zu | %s\n", #CLASS, CLASS::max_size(), DOC)

int main() {
  printf("%-23s | %-5s | %s\n", "class", "len", "comment");
  doc(StringPack::Raw,         "Fastest (no conversion applied)");
  doc(StringPack::ASCII,       "Fast (chars >= 128 will be converted to 127)");
  doc(StringPack::Alnum,       "");
  doc(StringPack::AlnumNoCase, "");
  doc(StringPack::Alpha,       "");
  doc(StringPack::AlphaNoCase, "");
  doc(StringPack::Upper,       "");
  doc(StringPack::Lower,       "");
  doc(StringPack::L33tNoCase,  "Like AlnumNoCase, but 0-9 are converted to 'OLZEASGTBQ'");
  doc(StringPack::Numeric,     "");
  doc(StringPack::Floatic,     "");

  {
    using SC = StringPack::Raw;

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
    using SC = StringPack::AlphaNoCase;
    test_conversion("ABC_def_GHI", "abc_DEF_ghi");
  }

  {
    using SC = StringPack::AlnumNoCase;
    test_conversion("aBc_123", "AbC_123");
  }

  {
    using SC = StringPack::Lower;

    assert(0  == SC::pack(""));
    assert(1  == SC::pack("a"));
    assert(26 == SC::pack("z"));
    assert(27 == SC::pack("_"));
    assert(27 == SC::pack("+"));

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

  {
    using SC = StringPack::Numeric;
    test(SC, "123456789");
  }

  {
    using SC = StringPack::Floatic;
    test(SC, "0.,123456789");
  }

  {
    using SC = StringPack::L33tNoCase;
    test_conversion("0123456789", "OLZEASGTBQ");
  }
}
