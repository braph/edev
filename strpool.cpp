#ifndef _STRPOOL_HPP
#define _STRPOOL_HPP

#include <string>

class StringPool {
private:
  std::string storage;
public:
  StringPool() : storage("", 1) {}
  size_t              add(const char*);
  inline const char*  get(size_t id)    { return storage.c_str() + id;   }
  inline void         reserve(size_t n) { storage.reserve(n);            }
  inline size_t       size()            { return storage.size();         }
  inline char*        data()            { return (char*) storage.data(); }
};
#endif

#include <cstring>

size_t StringPool :: add(const char *s) {
  if (!*s)
    return 0;

  size_t len_with_0 = std::strlen(s) + 1;
  size_t pos = storage.find(s, 1, len_with_0);
  if (pos != std::string::npos)
    return pos;

  pos = storage.size();
  storage.append(s, len_with_0);
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
