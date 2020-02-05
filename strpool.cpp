#ifndef _STRPOOL_HPP
#define _STRPOOL_HPP

#include <string>

class StringPool {
private:
  std::string storage;
public:
  StringPool() { storage.append("", 1); }
  size_t              add(const char*);
  inline const char*  get(size_t id)    { return storage.c_str() + id; }
  inline void         reserve(size_t n) { storage.reserve(n); }
};
#endif

#include <cstring>

size_t StringPool :: add(const char *s) {
  if (!*s)
    return 0;

  const size_t len = std::strlen(s) + 1;

#if 1 || CPP_STRFOO
  size_t pos = storage.find(s, 1, len);
  if (pos != std::string::npos)
    return pos;
#else
  size_t pos = 1;
  const size_t storage_len = storage.size();
  const char *cstr = storage.c_str();
  while (pos < storage_len) {
    const char *found = strstr(cstr+pos, s);
    if (found)
      return found - cstr;
    else
      pos += strlen(cstr) + 1;
  }
#endif

  pos = storage.size();
  storage.append(s, len);
  return pos;
}

#if TEST_STRPOOL
#include <cassert>
#include <iostream>
#define streq(A,B) (!strcmp(A,B))

#define TEST_DATA \
  {"", "1", "2", "3", "foo", "bar", "baz"}

int main() {
  try {
    StringPool pool;
    assert(streq("", pool.get(0)));
    size_t id0 = pool.add("0substr");
    size_t id1 = pool.add("substr");
    size_t id2 = pool.add("substr");
    assert(streq("0substr", pool.get(id0)));
    assert(streq("substr",  pool.get(id1)));
    assert(streq("substr",  pool.get(id2)));
    
    for (auto s : TEST_DATA)
      assert(streq(s, pool.get(pool.add(s))));

  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }
}
#endif
