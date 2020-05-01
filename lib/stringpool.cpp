#include "stringpool.hpp"

#include <cstring>
#include <climits>
#include <algorithm>

int StringPool :: add(CString s) {
  // No check for empty string since this should be the rare case and
  // find() will return pos `0` (the NUL byte at the beginning) in that case.

  const size_t pos = _store.find(s, 0, s.length() + 1);
  if (pos != std::string::npos)
    return pos;

  return add_unchecked(s);
}

int StringPool :: add_unchecked(CString s) {
  if (s.length()) {
    const size_t pos = _store.size();
    _store.append(s, s.length() + 1);
    return pos;
  }

  return 0;
}

int StringPool :: find(CString s, int start_pos) const noexcept {
  if (s.length()) {
    const size_t pos = _store.find(s, size_t(start_pos), s.length() + 1);
    if (pos != std::string::npos)
      return pos;
  }

  return 0;
}

int StringPool :: count() const noexcept {
  return std::count(_store.cbegin() + 1, _store.cend(), '\0');
}

bool StringPool :: is_shrinked() const noexcept {
  int len = 0;
  int endChar = 0;
  int last_len = INT_MAX;
  int last_endChar = 0;

  for (auto _c : _store) {
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

StringPool::Shrinker :: Shrinker(StringPool& pool)
: _pool(pool)
, _id_remap(_pool._store.size())
, _num_ids(0)
{
}

void StringPool::Shrinker :: add(int id) {
  if (! _id_remap[size_t(id)]) {
    ++_num_ids;
    _id_remap[size_t(id)] = id;
  }
}

void StringPool::Shrinker :: shrink_pool() {
  struct IDAndLength {
    int id;
    unsigned short length;
    unsigned char last_char;
  };

  Array<IDAndLength> ids_with_length(_num_ids);
  const char* pool_data = _pool._store.data();

  size_t i = 0;
  for (auto& id : _id_remap) {
    if (id) {
      const char* s = pool_data + id;
      const int len = std::strlen(s);
      if (len) {
        unsigned char last_char = reinterpret_cast<const unsigned char*>(s)[len-1];
        ids_with_length[i++] = {id, static_cast<unsigned short>(len), last_char};
      }
      else
        id = 0;
    }
  }

  // Sort by last_char(ascending) + length(descending)
#define key_func(S) ((unsigned(~S.last_char) << 24) + S.length)
  std::sort(ids_with_length.begin(), ids_with_length.end(),
      [](const IDAndLength& a, const IDAndLength& b){
        return key_func(a) > key_func(b);
  });

//for (auto i : ids_with_length) 
//  std::cout << int(i.last_char) << " [" << i.length << "]\n";

  StringPool new_pool;
  new_pool.reserve(size_t(_pool.size()));

  // Add strings in the right order to the stringpool and store the new ID
  int poolSearchPos = 0;
  unsigned char last_char = 0;

  for (auto& idAndLength : ids_with_length) {
    if (idAndLength.last_char != last_char) {
      poolSearchPos = new_pool.size();
      last_char = idAndLength.last_char;
    }

    const size_t id = static_cast<size_t>(idAndLength.id);
    CString s(pool_data + id, idAndLength.length);
    _id_remap[id] = new_pool.find(s, poolSearchPos);
    if (! _id_remap[id])
      _id_remap[id] = new_pool.add_unchecked(s);
  }

  _pool._store = std::move(new_pool._store);
  _pool._store.shrink_to_fit();
}

int StringPool::Shrinker :: get_new_id(int id) {
  return _id_remap[size_t(id)];
}

#ifdef F____
void StringPool :: shrink_to_fit(IDRemap& old_id_new_id) {
  StringPool newPool;
  newPool.reserve(_store.size());

  struct IDAndLength { int id; int length; };
  std::vector<IDAndLength> idAndLengthsByLastChar[256];

  // Assuming most of the strings end with [A-Za-z0-9]
  const size_t avg = old_id_new_id.size() / (26 + 26 + 10);
  for (int c = 'a'; c <= 'z'; ++c) idAndLengthsByLastChar[c].reserve(avg);
  for (int c = 'A'; c <= 'Z'; ++c) idAndLengthsByLastChar[c].reserve(avg);
  for (int c = '0'; c <= '9'; ++c) idAndLengthsByLastChar[c].reserve(avg);

  for (auto& pair : old_id_new_id) {
    const char* s = this->get(pair.first);
    const int len = std::strlen(s);
    if (len) {
      const int lastChar = reinterpret_cast<const unsigned char*>(s)[len-1];
      idAndLengthsByLastChar[lastChar].push_back({pair.first, len});
    }
    else {
      pair.second = 0;
    }
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
        CString s(this->get(idAndLength.id), size_t(idAndLength.length));
        int newId = newPool.find(s, poolSearchPos);
        if (!newId)
          newId = newPool.add_unchecked(s);
        old_id_new_id[idAndLength.id] = newId;
      }

      poolSearchPos = newPool.size();
    }

  _store = std::move(newPool._store);
  _store.shrink_to_fit();
}
#endif

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
