#ifndef _STRPOOL_HPP
#define _STRPOOL_HPP

#include <string>

/* First string of the pool is always an empty string "" with ID 0 */
class StringPool {
public:
  StringPool() : storage(1, '\0') {}

  /* Adds string `s` to the stringpool. If `force_append` is true, no attempts
   * are made to return an existing string in the string pool. */
  int add(const char* s, bool force_append = false);

  /* A pool is optimized when it it sorted by length in descending order. */
  bool isOptimized() const;

  /* Return the number of NUL terminated strings */
  int count() const;

  /* Returns the ID for string `s`.
   * If the string is empty or it could not be found, 0 is returned. */
  int find(const char *s) const;

  inline const char*  get(int id)      const { return storage.c_str() + id; }
  inline int          size()           const { return storage.size();       }
  inline int          capacity()       const { return storage.capacity();   }
  inline void         resize(size_t n)       { storage.resize(n);           }
  inline void         reserve(size_t n)      { storage.reserve(n);          }
  inline char*        data()                 { return const_cast<char*>(storage.data()); }
private:
  std::string storage;
};

#endif
