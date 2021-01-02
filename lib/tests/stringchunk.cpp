#include <lib/stringchunk.hpp>
#include <lib/test.hpp>

#define TEST_DATA \
  {"", "1", "2", "3", "foo", "bar", "baz"}

int main() {
  TEST_BEGIN();

  { /* Test: behaviour of empty strings */
    StringChunk chunk;
    assert(0 == chunk.find("non-existent"));
    assert(0 == chunk.add(""));
    assert(streq("", chunk.get(0)));
    assert(0 == chunk.count());
  }

  { /* Test: (sub-)string deduplication */
    StringChunk chunk;
    int id0 = chunk.add("0substr");
    int id1 = chunk.add("substr");
    int id2 = chunk.add("substr");
    assert(streq("0substr", chunk.get(id0)));
    assert(streq("substr",  chunk.get(id1)));
    assert(streq("substr",  chunk.get(id2)));
    assert(id1 == id2);
    assert(chunk.count() == 1);
  }

  {
    StringChunk chunk;
    for (auto s : TEST_DATA)
      assert(streq(s, chunk.get(chunk.add(s))));
    assert(chunk.count() == 6);
  }

  {
    StringChunk chunk;
    int id0 = chunk.add("string");
    int id1 = chunk.add("longstring");
    assert(! chunk.is_shrinked());

    auto shrinker = chunk.get_shrinker();
    shrinker.add(id0);
    shrinker.add(id1);
    shrinker.shrink();

    assert(chunk.is_shrinked());
  }

#ifdef TEST_STRINGCHUNK_PERFORMANCE
  {
    StringChunk chunk;
    for (int i = 0; i < 99999; ++i) chunk.add("performance", true);
    for (int i = 0; i < 99999; ++i) chunk.add("test", true);
    for (int i = 0; i < 1000; ++i) {
      chunk.count();
      chunk.is_shrinked();
    }
  }
#endif

  TEST_END();
}
