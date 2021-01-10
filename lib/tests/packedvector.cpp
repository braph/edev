#include "../test.hpp"
#include "../packedvector.hpp"
#include <vector>
#include <climits>
#include <cstdlib>

#define CHCK(...) do { \
    if (! (__VA_ARGS__)) throw std::runtime_error(#__VA_ARGS__); \
  } while(0)

#define RANGE(N) int tmp##__LINE__= 0; tmp##__LINE__ < N; ++tmp##__LINE__

template<typename TValue, typename TTestee, typename TExpect>
struct VectorTester {
  TTestee testee; // The vector TO be tested
  TExpect expect; // The reference vector

 ~VectorTester() { check_all(); }

#define proxy(NAME) \
  template<typename ... T> void NAME(T ... args) { \
    testee.NAME(args...); \
    expect.NAME(args...); \
  }

  proxy(clear)
  proxy(pop_back)
  proxy(push_back)
  proxy(emplace_back)
  proxy(reserve)
  proxy(resize)

  size_t size() { return expect.size(); }

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

  void check_empty()    { CHCK( testee.empty()    == expect.empty() ); }
  void check_size()     { CHCK( testee.size()     == expect.size()  ); }
  void check_capacity() { CHCK( testee.capacity() >= expect.capacity() ); }
  void check_front()    { if (!expect.empty()) CHCK( testee.front()    == expect.front() ); }
  void check_back()     { if (!expect.empty()) CHCK( testee.back()     == expect.back()  ); }

  void check_contents_by_index() {
    size_t sz = expect.size();
    for (size_t i = 0; i < sz; ++i)
      CHCK( testee[i] == expect[i] );
  }

  void check_contents_by_iterator() {
    auto testee_it = testee.begin(), testee_end = testee.end();
    auto expect_it = expect.begin(), expect_end = expect.end();

    while (testee_it != testee_end && expect_it != expect_end)
      CHCK( *testee_it++ == *expect_it++ );

    CHCK( testee_it == testee_end );
    CHCK( expect_it == expect_end );
  }

  void dump_contents_by_iterator() {
    auto testee_it = testee.begin(), testee_end = testee.end();
    auto expect_it = expect.begin(), expect_end = expect.end();

    while (testee_it != testee_end && expect_it != expect_end)
      printf("%10d %10d\n", *testee_it++, int(*expect_it++));
  }

  void check_all() {
    check_empty();
    check_size();
    check_front();
    check_back();
    check_contents_by_iterator();
    check_contents_by_index();
  }
};

static inline int rand(int max) { return rand() % max; }

int main() {
  TEST_BEGIN();

  int i;
  using V = VectorTester<int, DynamicPackedVector<int>, std::vector<int>>;

  { V v; v.check_all(); }

  { // push_back
    V v;
    for (RANGE(1024))
      v.push_back(i);
    v.check_contents_by_iterator();

    for (i = USHRT_MAX; i < USHRT_MAX + 1024; ++i)
      v.push_back(i);
    v.check_contents_by_iterator();
  }

#define MAX_VEC_SIZE 16000
#define MAX_VEC_VALUE UINT_MAX

#define case break;case
  for (RANGE(10)) {
    V v;
    v.check_all();

    int op = 0, arg = 0, arg2 = 0;
    for (RANGE(10000)) {
      switch ((op = rand(100))) {
      default: v.push_back(arg = rand(MAX_VEC_VALUE));
      case 0:  v.reserve(arg = rand(MAX_VEC_SIZE));
      case 1:  v.resize(arg = rand(MAX_VEC_SIZE), arg2 = rand(MAX_VEC_VALUE));
      case 2:  v.clear();
      case 3:  if (v.size()) v.pop_back();
      }

      try { v.check_all(); }
      catch (...) {
        printf("OP=%d arg=%d arg2=%d\n", op, arg, arg2);
        throw;
      }
    }
  }
#undef case

  { // emplace_back
    V v;
    for (int i = 0; i < 1024; ++i)                 v.emplace_back(i);
    for (i = USHRT_MAX; i < USHRT_MAX + 1024; ++i) v.emplace_back(i);

    v.check_contents_by_iterator();

#if 0
    for (int i = 0; i <= INT_MAX; i *= 2)
      v.push_back(i);
    v.check_all();
    v.clear();
#endif
  }

#if 0
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
#endif

  TEST_END();
}
