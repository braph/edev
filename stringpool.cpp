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

int StringPool :: find(const char *s) const {
  if (! *s)
    return 0;

  size_t pos = storage.find(s, 1 /* Skip empty string */, std::strlen(s) + 1);
  if (pos != std::string::npos)
    return pos;

  return 0;
}

bool StringPool :: isOptimized() const {
  const char* pool_data = storage.data();
  int last_len = INT_MAX;
  int len;

  for (int i = 1 /* Skip empty string */; i < size();) {
    len = std::strlen(pool_data + i);
    if (len > last_len)
      return false;
    last_len = len;
    i += len + 1;
  }

  return true;
}

int StringPool :: count() const {
  const char* pool_data = storage.data() + 1;
  const char* pool_end =  storage.data() + storage.size();

  size_t n = 0;
  while (pool_data < pool_end)
    pool_data += std::strlen(pool_data) + 1, ++n;

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

  TEST_END();
}
#endif
