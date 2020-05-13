#ifndef LIB_STRINGCHUNK_HPP
#define LIB_STRINGCHUNK_HPP

#include "cstring.hpp"
#include "array.hpp"

#include <string>

class StringChunk {
public:
  /* First string in the chunk is always an empty string "" with ID 0 */
  StringChunk() : _data(1, '\0') {}

  /* Adds string `s` to the stringchunk.
   * If the string already exists in the chunk, its ID will be returned and
   * no new insertion is made. */
  int add(CString s);

  /* Adds string `s` to the stringchunk.
   * No attempts are made to return an existing string from the chunk. */
  int add_unchecked(CString s);

  /* Returns the ID for string `s`.
   * If the string is empty or it could not be found, 0 is returned. */
  int find(CString s) const noexcept { return find(s, 1); }

  /* Return the number of NUL terminated strings */
  int count() const noexcept;

  char const* get(int id) const noexcept { return _data.c_str() + id; }
  int         size()      const noexcept { return _data.size();       }
  int         capacity()  const noexcept { return _data.capacity();   }
  char*       data()            noexcept { return const_cast<char*>(_data.data()); }
  void        resize(size_t n)           { _data.resize(n);  }
  void        reserve(size_t n)          { _data.reserve(n); }

  struct Shrinker {
    void add(int id);
    int  get_new_id(int old_id);
    void shrink();
  private:
    friend class StringChunk;
    Shrinker(StringChunk&);
    StringChunk& _chunk;
    Array<int> _id_remap;
    size_t _num_ids;
  };

  Shrinker get_shrinker() { return Shrinker(*this); }
  bool is_shrinked() const noexcept;

private:
  std::string _data;
  int find(CString s, int) const noexcept;
};

#endif
