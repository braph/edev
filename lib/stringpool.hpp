#ifndef STRINGPOOL_HPP
#define STRINGPOOL_HPP

#include <string>
#include <unordered_map>

class StringPool {
public:
  /* First string in the pool is always an empty string "" with ID 0 */
  StringPool() : storage(1, '\0') {}

  /* Adds string `s` to the stringpool. If `force_append` is true, no attempts
   * are made to return an existing string from the string pool. */
  int add(const char* s, bool force_append = false);

  /* Returns the ID for string `s`.
   * If the string is empty or it could not be found, 0 is returned. */
  int find(const char* s) const noexcept  { return find(s, 1); }

  /* Return the number of NUL terminated strings */
  int count() const noexcept;

  /* Shrink to fit */
  void shrink_to_fit(std::unordered_map<int, int>&);
  bool is_shrinked() const noexcept;

  const char*  get(int id) const noexcept { return storage.c_str() + id; }
  int          size()      const noexcept { return storage.size();       }
  int          capacity()  const noexcept { return storage.capacity();   }
  char*        data()            noexcept { return const_cast<char*>(storage.data()); }
  void         resize(size_t n)           { storage.resize(n);  }
  void         reserve(size_t n)          { storage.reserve(n); }

private:
  std::string storage;
  int find(const char* s, int) const noexcept;
};

#endif
