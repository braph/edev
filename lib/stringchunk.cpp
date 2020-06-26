#include "stringchunk.hpp"

#include <cstring>
#include <climits>
#include <algorithm>

int StringChunk :: add(CString s) {
  // No check for empty string since this should be the rare case and
  // find() will return pos `0` (the NUL byte at the beginning) in that case.

  const size_t pos = _data.find(s, 0, s.length() + 1);
  if (pos != std::string::npos)
    return pos;

  return add_unchecked(s);
}

int StringChunk :: add_unchecked(CString s) {
  if (s.length()) {
    const size_t pos = _data.size();
    _data.append(s, s.length() + 1);
    return pos;
  }

  return 0;
}

int StringChunk :: find(CString s, int start_pos) const noexcept {
  if (s.length()) {
    const size_t pos = _data.find(s, size_t(start_pos), s.length() + 1);
    if (pos != std::string::npos)
      return pos;
  }

  return 0;
}

int StringChunk :: count() const noexcept {
  return std::count(_data.cbegin() + 1, _data.cend(), '\0');
}

bool StringChunk :: is_shrinked() const noexcept {
  int len = 0;
  int endChar = 0;
  int last_len = INT_MAX;
  int last_endChar = 0;

  for (auto _c : _data) {
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

StringChunk::Shrinker :: Shrinker(StringChunk& chunk)
: _chunk(chunk)
, _id_remap(_chunk._data.size())
, _num_ids(0)
{
}

void StringChunk::Shrinker :: add(int id) {
  if (! _id_remap[size_t(id)]) {
    ++_num_ids;
    _id_remap[size_t(id)] = id;
  }
}

void StringChunk::Shrinker :: shrink() {
  struct IDAndLength {
    int id;
    unsigned short length;
    unsigned char last_char;
  };

  HeapArray<IDAndLength> ids_with_length(_num_ids);
  const char* chunk_data = _chunk._data.data();

  size_t i = 0;
  for (auto& id : _id_remap) {
    if (id) {
      const char* s = chunk_data + id;
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

  StringChunk new_chunk;
  new_chunk.reserve(size_t(_chunk.size()));

  // Add strings in the right order to the stringchunk and store the new ID
  int chunkSearchPos = 0;
  unsigned char last_char = 0;

  for (auto& idAndLength : ids_with_length) {
    if (idAndLength.last_char != last_char) {
      chunkSearchPos = new_chunk.size();
      last_char = idAndLength.last_char;
    }

    const size_t id = static_cast<size_t>(idAndLength.id);
    CString s(chunk_data + id, idAndLength.length);
    _id_remap[id] = new_chunk.find(s, chunkSearchPos);
    if (! _id_remap[id])
      _id_remap[id] = new_chunk.add_unchecked(s);
  }

  _chunk._data = std::move(new_chunk._data);
  _chunk._data.shrink_to_fit();
}

int StringChunk::Shrinker :: get_new_id(int id) {
  return _id_remap[size_t(id)];
}

#ifdef F____
void StringChunk :: shrink_to_fit(IDRemap& old_id_new_id) {
  StringChunk newchunk;
  newchunk.reserve(_data.size());

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

  // Add strings in the right order to the stringchunk and store the new ID
  int chunkSearchPos = 0;
  for (const auto& idAndLengths : idAndLengthsByLastChar)
    if (idAndLengths.size()) {
      for (const auto& idAndLength : idAndLengths) {
        CString s(this->get(idAndLength.id), size_t(idAndLength.length));
        int newId = newchunk.find(s, chunkSearchPos);
        if (!newId)
          newId = newchunk.add_unchecked(s);
        old_id_new_id[idAndLength.id] = newId;
      }

      chunkSearchPos = newchunk.size();
    }

  _data = std::move(newchunk._data);
  _data.shrink_to_fit();
}
#endif

#ifdef TEST_STRINGchunk
#include "test.hpp"

#define TEST_DATA \
  {"", "1", "2", "3", "foo", "bar", "baz"}

int main() {
  TEST_BEGIN();

  StringChunk chunk;
  assert(! chunk.find("non-existent"));
  assert(streq("", chunk.get(0)));
  assert(0 == chunk.add(""));
  int id0 = chunk.add("0substr");
  int id1 = chunk.add("substr");
  int id2 = chunk.add("substr");
  assert(streq("0substr", chunk.get(id0)));
  assert(streq("substr",  chunk.get(id1)));
  assert(streq("substr",  chunk.get(id2)));
  assert(chunk.count() == 1);
  
  for (auto s : TEST_DATA)
    assert(streq(s, chunk.get(chunk.add(s))));

  assert(chunk.count() == 7);
  assert(! chunk.is_shrinked());

  // XXX These tests have to be changed to the new shrink_to_fit algo
  StringChunk optimized;
  optimized.add("longstring");
  optimized.add("short");
  assert(optimized.is_shrinked());
  optimized.add("longstring", true);
  assert(! optimized.is_shrinked());

#ifdef PERFORMANCE_TEST
  StringChunk perf;
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
