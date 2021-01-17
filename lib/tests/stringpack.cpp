#include "../stringpack.hpp"

#include <cassert>
#include <cstdio>

#define len(S) (sizeof(S) - 1)

// Check if all pack algorithms give the same result
#define test(str) \
  assert(SC::pack_runtime(str)           == SC::pack(str)); \
  assert(SC::pack_runtime(str, len(str)) == SC::pack(str))

#define test_is_overflow(str) \
  assert(SC::pack_runtime(str)           == StringPack::overflow); \
  assert(SC::pack_runtime(str, len(str)) == StringPack::overflow)

#define test_switch_case(str_1, str_2) \
  assert(SC::pack_runtime(str_1)             == SC::pack(str_2)); \
  assert(SC::pack_runtime(str_1, len(str_1)) == SC::pack(str_2))

#define doc(CLASS, DOC) \
  printf("%-23s | %-5zu | %s\n", #CLASS, CLASS::max_strlen(), DOC)

int main() {

#if 0
  auto x = StringPack::make_ignore_t(0,0,0,0, 1, 96);
  auto x = StringPack::make_ignore_t<0,0,0,0, 'a', 1>::value();
  printf("%lu %lu %lu %lu\n", x.ign[3], x.ign[2], x.ign[1], x.ign[0]);
  printf("%lu\n", StringPack::ignore<'a','b', 255>(255));
  return StringPack::ignore<'a','b'>('a');
#endif

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
    using SC = StringPack::packer<
      StringPack::combine<
        StringPack::skip_chars<'_'>,
        StringPack::fast::raw
    >>;

    printf("foo:%d\n", SC::max_strlen());

    test("a");
    test("ab");
    test_switch_case("1_2_3_4_5_6_7_8", "12345678");
  }

  {
    using SC = StringPack::Raw;

    assert(0   == SC::pack(""));
    assert('a' == SC::pack("a"));
    test_is_overflow("overflow0");

    test("");
    test("a");
    test("aa");
    test("aaa");
    test("aaaa");
    test("aaaaa");
    test("aaaaaa");
    test("aaaaaaa");
    test("aaaaaaaa");
    test("ab");
    test("abc");
    test("abcd");
    test("abcde");
    test("abcdef");
    test("abcdefg");
    test("abcdefgh");
    test("b");
    test("x");
  }

  {
    using SC = StringPack::ASCII;
    assert(0   == SC::pack(""));
    assert(127 == SC::pack("\x7F"));
    assert(127 == SC::pack("\x80"));

    test("nine char");
    test_is_overflow("overflow10");
  }

  {
    using SC = StringPack::AlphaNoCase;
/*
    printf(">>>>%d\n", SC::char_count());
    for (int i = 0; i < 255; ++i)
      printf(">>>> %c, %d\n", i, StringPack::fast::alpha_nocase(i));
*/
    test_switch_case("abc_DEF_ghi", "ABC_def_GHI");
  }

  {
    using SC = StringPack::AlnumNoCase;
    test_switch_case("AbC_123", "aBc_123");
  }

  {
    using SC = StringPack::Lower;

    assert(0  == SC::pack(""));
    assert(1  == SC::pack("a"));
    assert(26 == SC::pack("z"));
    printf(">>>>>>>>>>>>>>%d\n", SC::pack("_"));
    assert(27 == SC::pack("_"));
    assert(27 == SC::pack("+"));

    test_is_overflow("overflow_overflow");

    test("");

    test("a");
    test("aa");
    test("aaa");
    test("aaaa");
    test("aaaaa");
    test("aaaaaa");
    test("aaaaaaa");
    test("aaaaaaaa");

    test("ab");
    test("abc");
    test("abcd");
    test("abcde");
    test("abcdef");
    test("abcdefg");
    test("abcdefgh");
    test("b");
    test("x");
  }

  {
    using SC = StringPack::Numeric;
    test("123456789");
  }

  {
    using SC = StringPack::Floatic;
/*
    printf(">>>>%d\n", SC::char_count());
    for (int i = 0; i < 255; ++i)
      printf(">>>> %c, %d\n", i, StringPack::fast::floatic(i));
*/
    test("0.,123456789");
  }

  {
    using SC = StringPack::L33tNoCase;
    test_switch_case("OLZEASGTBQ", "0123456789");
  }
}
