#ifndef _STRPOOL_HPP
#define _STRPOOL_HPP

#include <string>

/* First string of the pool is always an empty string "" with ID 0 */
class StringPool {
public:
  StringPool() : storage(1, '\0') {}

  /* Adds string `s` to the stringpool. If `force_append` is true, no attempts
   * are made to return an existing string in the string pool. */
  size_t add(const char* s, bool force_append = false);

  /* A pool is optimized when it it sorted by length (descending). */
  bool isOptimized() const;

  /* Return the number of NUL terminated strings */
  size_t count() const;

  /* Returns the ID for string `s`.
   * If the string is empty or it could not be found, 0 is returned. */
  size_t find(const char *s) const;

  inline const char*  get(size_t id)   const { return storage.c_str() + id; }
  inline size_t       size()           const { return storage.size();       }
  inline size_t       capacity()       const { return storage.capacity();   }
  inline void         resize(size_t n)       { storage.resize(n);           }
  inline void         reserve(size_t n)      { storage.reserve(n);          }
  inline char*        data()                 { return const_cast<char*>(storage.data()); }
private:
  std::string storage;
};

#endif
