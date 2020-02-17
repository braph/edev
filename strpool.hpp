#ifndef _STRPOOL_HPP
#define _STRPOOL_HPP

#include <string>

class StringPool {
public:
  StringPool() : storage("", 1) {}
  size_t              add(const char*, bool *newly_inserted=NULL);
  inline const char*  get(size_t id)    { return storage.c_str() + id;   }
  inline void         reserve(size_t n) { storage.reserve(n);            }
  inline void         resize(size_t n)  { storage.resize(n);             }
  inline size_t       size()            { return storage.size();         }
  inline size_t       capacity()        { return storage.capacity();     }
  inline char*        data()            { return const_cast<char*>(storage.data()); }
  std::string&        storage2()        { return storage;                }
private:
  std::string storage;
};

#endif
