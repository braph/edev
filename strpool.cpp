#include "strpool.hpp"
#include <cstring>

/* Adds string `s` to the stringpool.
 * If the pool didn't contain the string yet and `newly_inserted` is not NULL,
 * it will be set to `true`.  NB: It's up to the caller to initialize the
 * variable to `false` before the call. */
size_t StringPool :: add(const char *s, bool *newly_inserted) {
  if (!*s)
    return 0;

  size_t len_with_0 = std::strlen(s) + 1;
  size_t pos = storage.find(s, 1, len_with_0);
  if (pos != std::string::npos)
    return pos;

  if (newly_inserted)
    *newly_inserted = true;
  pos = storage.size();
  storage.append(s, len_with_0);
  return pos;
}

#if TEST_STRPOOL
#include "test.hpp"

#define TEST_DATA \
  {"", "1", "2", "3", "foo", "bar", "baz"}

int main() {
  TEST_BEGIN

  StringPool pool;
  assert(streq("", pool.get(0)));
  assert(0 == pool.add(""));
  size_t id0 = pool.add("0substr");
  size_t id1 = pool.add("substr");
  size_t id2 = pool.add("substr");
  assert(streq("0substr", pool.get(id0)));
  assert(streq("substr",  pool.get(id1)));
  assert(streq("substr",  pool.get(id2)));
  
  for (auto s : TEST_DATA)
    assert(streq(s, pool.get(pool.add(s))));

  TEST_END
}
#endif
