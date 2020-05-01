#ifndef LIB_STRINGPOOL_HPP
#define LIB_STRINGPOOL_HPP

#include "cstring.hpp"
#include "array.hpp"

#include <string>

class StringPool {
public:
  /* First string in the pool is always an empty string "" with ID 0 */
  StringPool() : _store(1, '\0') {}

  /* Adds string `s` to the stringpool.
   * If the string already exists in the pool, its ID will be returned and
   * no new insertion is made. */
  int add(CString s);

  /* Adds string `s` to the stringpool.
   * No attempts are made to return an existing string from the pool. */
  int add_unchecked(CString s);

  /* Returns the ID for string `s`.
   * If the string is empty or it could not be found, 0 is returned. */
  int find(CString s) const noexcept { return find(s, 1); }

  /* Return the number of NUL terminated strings */
  int count() const noexcept;

  char const* get(int id) const noexcept { return _store.c_str() + id; }
  int         size()      const noexcept { return _store.size();       }
  int         capacity()  const noexcept { return _store.capacity();   }
  char*       data()            noexcept { return const_cast<char*>(_store.data()); }
  void        resize(size_t n)           { _store.resize(n);  }
  void        reserve(size_t n)          { _store.reserve(n); }

  struct Shrinker {
    void add(int id);
    int  get_new_id(int old_id);
    void shrink_pool();
  private:
    friend class StringPool;
    Shrinker(StringPool&);
    StringPool& _pool;
    Array<int> _id_remap;
    size_t _num_ids;
  };

  /* Shrink to fit XXX */
  Shrinker get_shrinker() { return Shrinker(*this); }
  bool is_shrinked() const noexcept;

private:
  std::string _store;
  int find(CString s, int) const noexcept;
};

#endif
