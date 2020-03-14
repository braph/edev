#include "stringpool.hpp"

#include <vector>
#include <cstring>
#include <climits>
#include <algorithm>

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

int StringPool :: find(const char *s, int start_pos) const noexcept {
  if (! *s)
    return 0;

  size_t pos = storage.find(s, size_t(start_pos), std::strlen(s) + 1);
  if (pos != std::string::npos)
    return pos;

  return 0;
}

bool StringPool :: is_shrinked() const noexcept {
  int len = 0;
  int endChar = 0;
  int last_len = INT_MAX;
  int last_endChar = 0;

  for (const auto& _c : storage) {
    unsigned char c = static_cast<unsigned char>(_c);

    if (c) {
      endChar = c;
      ++len;
    }
    else {
      if (endChar < last_endChar)
        return false;
      else if (endChar > last_endChar) {
        last_endChar = endChar;
        last_len = len;
        len = 0;
      }
      else {
        if (len > last_len)
          return false;

        last_len = len;
        len = 0;
      }
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

void StringPool :: shrink_to_fit(std::unordered_map<int, int>& old_id_new_id) {
  StringPool newPool;
  newPool.reserve(size_t(size()));

  struct IDAndLength { int id; int length; };
  std::vector<IDAndLength> idAndLengthsByLastChar[256];

  // Assuming that most of the strings end with [A-Za-z0-9]
  const size_t avg = old_id_new_id.size() / ( 26 + 26 + 10 );
  for (int c = 'a'; c <= 'z'; ++c) idAndLengthsByLastChar[c].reserve(avg);
  for (int c = 'A'; c <= 'Z'; ++c) idAndLengthsByLastChar[c].reserve(avg);
  for (int c = '0'; c <= '9'; ++c) idAndLengthsByLastChar[c].reserve(avg);

  for (const auto& pair : old_id_new_id) {
    const char* s = this->get(pair.first);
    const int len = std::strlen(s);
    const int lastChar = (len ? reinterpret_cast<const unsigned char*>(s)[len-1] : 0);
    idAndLengthsByLastChar[lastChar].push_back({pair.first, len});
  }

  // Sort by length
  for (auto& idAndLengths : idAndLengthsByLastChar)
    if (idAndLengths.size())
      std::sort(idAndLengths.begin(), idAndLengths.end(),
          [](const IDAndLength& a, const IDAndLength& b){ return a.length > b.length; });

  // Add strings in the right order to the stringpool and store the new ID
  int poolSearchPos = 0;
  for (const auto& idAndLengths : idAndLengthsByLastChar)
    if (idAndLengths.size()) {
      for (const auto& idAndLength : idAndLengths) {
        const char* s = this->get(idAndLength.id);
        int newId = newPool.find(s, poolSearchPos);
        if (! newId)
          newId = newPool.add(s, true);
        old_id_new_id[idAndLength.id] = newId;
      }

      poolSearchPos = newPool.size();
    }

  storage = std::move(newPool.storage);
  storage.shrink_to_fit();
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
  assert(! pool.is_shrinked());

  // XXX These tests have to be changed to the new shrink_to_fit algo
  StringPool optimized;
  optimized.add("longstring");
  optimized.add("short");
  assert(optimized.is_shrinked());
  optimized.add("longstring", true);
  assert(! optimized.is_shrinked());

#ifdef PERFORMANCE_TEST
  StringPool perf;
  for (int i = 0; i < 99999; ++i) perf.add("performance", true);
  for (int i = 0; i < 99999; ++i) perf.add("test", true);
  for (int i = 0; i < 1000; ++i) {
    perf.count();
    perf.is_shrinked();
  }
#endif

  TEST_END();
}
#endif
