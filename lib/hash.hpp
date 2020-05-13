#ifndef LIB_HASH
#define LIB_HASH

/* Some constexpr hash algorithms.
 * Taken from http://www.cse.yorku.ca/~oz/hash.html
 */

namespace Hash {

template<typename TInt>
constexpr TInt lose_lose(const char* s) {
  return
    (*s) ? (
      lose_lose<TInt>(s + 1) + TInt(*s)
    ) : (
      0
    );
}

} // namespace Hash

#endif
