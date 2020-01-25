#ifndef _STRPOOL_HPP
#define _STRPOOL_HPP

#include <string>
#include <vector>

class StringPool {
  private:
    int m_chunkSize;
    std::vector<std::string> m_chunks;

  public:
    StringPool(int chunkSize = 512);
    const char* add(const char*);
};
#endif

#include <cstring>

StringPool :: StringPool(int chunkSize) : m_chunkSize(chunkSize) { }

const char* StringPool :: add(const char *s) {
  const size_t len = std::strlen(s);

  /* Optimize for inserting sorted strings (with duplicates):
   * It is more likely to find the last inserted string on an string chunk
   * thats located at the end. */
  for (auto chunk = m_chunks.crbegin(); chunk != m_chunks.crend(); ++chunk) {
    const size_t pos = chunk->find(s, 0, len+1 /* include '\0' */);
    if (pos != std::string::npos)
      return chunk->c_str() + pos; // Pool contains (sub)string
  }

  for (auto &chunk : m_chunks) {
    const size_t size     = chunk.size();
    const size_t capacity = chunk.capacity();
    if (len + 1 < capacity - size) {
      chunk.resize(size + len + 1, '\0'); // Make room for new value
      chunk.replace(size, len, s, len);
      return chunk.c_str() + size;
    }
  }

  m_chunks.push_back(s);
  std::string &val = m_chunks.back();
  val.reserve(m_chunkSize);  // Prevent reallocation
  val.resize(len + 1, '\0'); // Make string NUL terminated
  return val.c_str();
}

#if TEST_STRPOOL
#include <cassert>
#include <iostream>
#define streq(A,B) (!strcmp(A,B))

#define TEST_DATA \
  {"", "1", "2", "3", "foo", "bar", "baz"}

void test(int chunkSize) {
  StringPool pool(chunkSize);
  for (auto s : TEST_DATA)
    assert( streq(s, pool.add(s)) );

  /* Check if substring deduplication works */
  const char *s0 = pool.add("0substr");
  const char *s1 = pool.add("substr");
  assert( s1-1 == s0 );
}

int main() {
  try {
    for (int i = 0; i < 64; ++i)
      test(i);
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }
}
#endif
