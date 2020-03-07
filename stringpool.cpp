#include "stringpool.hpp"

#include <cstring>
#include <climits>

int StringPool :: add(const char *s, bool force_append) {
  if (! *s)
    return 0;

  if (! force_append) {
    int existing = find(s);
    if (existing)
      return existing;
  }

  size_t pos = storage.size();
  storage.append(s, std::strlen(s) + 1);
  return pos;
}

int StringPool :: find(const char *s) const noexcept {
  if (! *s)
    return 0;

  size_t pos = storage.find(s, 1 /* Skip empty string */, std::strlen(s) + 1);
  if (pos != std::string::npos)
    return pos;

  return 0;
}

bool StringPool :: isOptimized() const noexcept {
  int last_len = INT_MAX;
  int len = 0;

  for (auto it = storage.cbegin()+1; it != storage.cend(); ++it) {
    if (!*it) {
      if (len > last_len)
        return false;
      last_len = len;
      len = 0;
    }
    else {
      len++;
    }
  }

  return true;
}

int StringPool :: count() const noexcept {
  int n = 0;
  for (auto c : storage)
    if (!c)
      ++n;
  return n-1;
}

#ifdef TEST_STRINGPOOL
#include "test.hpp"

#define TEST_DATA \
  {"", "1", "2", "3", "foo", "bar", "baz"}

int main() {
  TEST_BEGIN();

  StringPool pool;
  assert(! pool.find("non-existent"));
  assert(streq("", pool.get(0)));
  assert(0 == pool.add(""));
  int id0 = pool.add("0substr");
  int id1 = pool.add("substr");
  int id2 = pool.add("substr");
  assert(streq("0substr", pool.get(id0)));
  assert(streq("substr",  pool.get(id1)));
  assert(streq("substr",  pool.get(id2)));
  assert(pool.count() == 1);
  
  for (auto s : TEST_DATA)
    assert(streq(s, pool.get(pool.add(s))));

  assert(pool.count() == 7);
  assert(! pool.isOptimized());

  StringPool optimized;
  optimized.add("longstring");
  optimized.add("short");
  assert(optimized.isOptimized());
  optimized.add("longstring", true);
  assert(! optimized.isOptimized());

#ifdef PERFORMANCE_TEST
  StringPool perf;
  for (int i = 0; i < 99999; ++i) perf.add("performance", true);
  for (int i = 0; i < 99999; ++i) perf.add("test", true);
  for (int i = 0; i < 1000; ++i) {
    perf.count();
    perf.isOptimized();
  }
#endif

  TEST_END();
}
#endif
