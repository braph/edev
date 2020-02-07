#ifndef _STRPOOL_HPP
#define _STRPOOL_HPP

#include <string>

class StringPool {
private:
  std::string storage;
public:
  StringPool() : storage("", 1) {}
  size_t              add(const char*, bool *newly_inserted=NULL);
  inline const char*  get(size_t id)    { return storage.c_str() + id;   }
  inline void         reserve(size_t n) { storage.reserve(n);            }
  inline size_t       size()            { return storage.size();         }
  inline char*        data()            { return (char*) storage.data(); }
};
#endif
