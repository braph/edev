#include "stringpool.hpp"

#include <cstring>
#include <climits>

size_t StringPool :: add(const char *s, bool force_append) {
  if (!*s)
    return 0;

  if (! force_append) {
    size_t existing = find(s);
    if (existing)
      return existing;
  }

  size_t pos = storage.size();
  storage.append(s, std::strlen(s) + 1);
  return pos;
}

size_t StringPool :: find(const char *s) const {
  if (!*s)
    return 0;

  size_t pos = storage.find(s, 1 /* Skip empty string */, std::strlen(s) + 1);
  if (pos != std::string::npos)
    return pos;

  return 0;
}

bool StringPool :: isOptimized() const {
  const char* pool_data = storage.data();
  size_t pool_size = size();
  size_t last = INT_MAX;
  size_t len;

  for (size_t i = 1; /* Skip empty string */ i < pool_size;) {
    len = strlen(pool_data + i);
    if (len > last)
      return false;
    last = len;
    i += len + 1;
  }

  return true;
}

size_t StringPool :: count() const {
  const char* pool_data = storage.data() + 1;
  const char* pool_end =  storage.data() + storage.size();

  size_t n = 0;
  while (pool_data < pool_end)
    pool_data += strlen(pool_data) + 1, ++n;

  return n;
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
  size_t id0 = pool.add("0substr");
  size_t id1 = pool.add("substr");
  size_t id2 = pool.add("substr");
  assert(streq("0substr", pool.get(id0)));
  assert(streq("substr",  pool.get(id1)));
  assert(streq("substr",  pool.get(id2)));
  assert(pool.count() == 1);
  
  for (auto s : TEST_DATA)
    assert(streq(s, pool.get(pool.add(s))));

  assert(pool.count() == 7);
  assert(!pool.isOptimized());

  StringPool optimized;
  optimized.add("longstring");
  optimized.add("short");
  assert(optimized.isOptimized());
  optimized.add("longstring", true);
  assert(! optimized.isOptimized());

  TEST_END();
}
#endif
