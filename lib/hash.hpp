#ifndef LIB_HASH
#define LIB_HASH

#include <cstdint>

/* Some constexpr hash algorithms.
 * Taken from http://www.cse.yorku.ca/~oz/hash.html
 */

namespace Hash {

constexpr uint64_t lose_lose(const char* s) noexcept {
  return
    (*s) ? (
      lose_lose(s + 1) + uint64_t(*s)
    ) : (
      0
    );
}

constexpr uint64_t djb2(const char* s) noexcept {
  return
    (*s) ? (
      djb2(s + 1) * 33 + uint64_t(*s)
    ) : (
      5381
    );
}

constexpr uint64_t sdbm(const char* s) noexcept {
  return
    (*s) ? (
      sdbm(s + 1) * 65599 + uint64_t(*s)
    ) : (
      0
    );
}

template<class T> uint64_t lose_lose(const T& s) noexcept { return lose_lose(s.c_str()); }
template<class T> uint64_t djb2     (const T& s) noexcept { return djb2(s.c_str());      }
template<class T> uint64_t sdbm     (const T& s) noexcept { return sdbm(s.c_str());      }

} // namespace Hash

#endif
