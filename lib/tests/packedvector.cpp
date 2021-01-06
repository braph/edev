#include "../test.hpp"
#include "../packedvector.hpp"
#include <vector>
#include <initializer_list>

/**
 * Foo
 */
template<typename TValue, typename TTestee, typename TExpect>
struct VectorTester {
  using value_type = TValue;

  TTestee testee; // The vector TO be tested
  TExpect expect; // The reference vector

#define proxy0(F_RET, F_NAME) \
  F_RET F_NAME() { \
    testee.F_NAME(); \
    expect.F_NAME(); }

#define proxy1(F_RET, F_NAME, A1_TYPE, A1_NAME) \
  F_RET F_NAME(A1_TYPE A1_NAME) { \
    testee.F_NAME(A1_NAME); \
    expect.F_NAME(A1_NAME); }

  proxy0(void,clear)
  proxy0(void,pop_back)
  proxy1(void,push_back,value_type,v)
  proxy1(void,reserve,size_t,n)
  proxy1(void,resize,size_t,n)

  struct reference {
    TTestee& testee;
    TExpect& expect;
    size_t pos;

    reference(TTestee& testee, TExpect &expect, size_t pos)
      : testee(testee)
      , expect(expect)
      , pos(pos)
    {}

    template<typename TRhsValue>
    reference& operator=(const TRhsValue& v) {
      testee[pos] = v;
      expect[pos] = v;
      return *this;
    }
  };

  reference operator[](size_t pos) {
    return reference(testee, expect, pos);
  }

#define _check(...) \
  if (! (__VA_ARGS__)) throw std::runtime_error(#__VA_ARGS__)

  void check_empty()    { _check( testee.empty()    == expect.empty() ); }
  void check_size()     { _check( testee.size()     == expect.size()  ); }
  void check_front()    { _check( testee.front()    == expect.front() ); }
  void check_back()     { _check( testee.back()     == expect.back()  ); }
  void check_capacity() { _check( testee.capacity() == expect.capacity() ); }
  void check_equals_using_index_access() {
    size_t sz = expect.size();
    for (size_t i = 0; i < sz; ++i)
      _check( testee[i] == expect[i] );
  }
  void check_equals_using_iterator_access() {
    auto testee_it = testee.begin(), testee_end = testee.end();
    auto expect_it = expect.begin(), expect_end = expect.end();

    while (testee_it != testee_end && expect_it != expect_end)
      _check( *testee_it++ == *expect_it++ );

    _check( testee_it == testee_end );
    _check( expect_it == expect_end );
  }

 ~VectorTester() {
   check_empty();
   check_size();
   check_front();
   check_back();
   check_equals_using_iterator_access();
   check_equals_using_index_access();
 }
};

int main() {
  TEST_BEGIN();

  using V = VectorTester<int, std::vector<int>, DynamicPackedVector>;

  { V v; } //assert(v.testee.capacity() == 0);

  {
    V v; // push_back
    for (int i = 0; i < 1024; ++i)
      v.push_back(i);
  }

  { V v; // emplace_back
    for (int i = 0; i < 1024; ++i)
      v.emplace_back(i); }

    // Clear
    v.clear();
    v.check_all();

    // Foo
    for (int i = 0; i <= INT_MAX; i *= 2)
      v.push_back(i);
    v.check_all();
    v.clear();

    // Direct access (get)
    for (int i = 0) {}

    // Direct access (set)
    for (int i = 0; i < TODO; ++i)
      v[i] = 2;
    v.check_all();

  }

#define SZ 1048576
  {
    std::cerr << "*iterator =\n";
    for (DynamicPackedVector::iterator it = v.begin(); it != v.end(); ++it) {
      *it = 33;
    }

    std::cerr << "get[33]\n";
    for (size_t i = 0; i < SZ; ++i) {
      assert(v[i] == 33);
    }
  }

  TEST_END();
}
