#include "stringchunk.hpp"

#include <cstring>
#include <climits>
#include <algorithm>

// StringChunk ================================================================

int StringChunk :: add(CString s) {
  // No check for empty string since find() will return pos `0` (the NUL byte
  // at the beginning) in that case.

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

// StringChunk :: Shrinker ====================================================

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

#ifdef TEST_STRINGCHUNK
#include "test.hpp"

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
#endif
