#ifndef SSCAN_HPP
#define SSCAN_HPP

#include <cstdlib>
#include <climits>
#include <cstring>
#include <cerrno>
#include <cinttypes>

/**
 * Helper for scanning a string.
 *
 * Provides methods for extracting numbers, skipping chars or whole strings.
 *
 * If a read failed, bool(SScan) will return false;
 * Use `error()` to return the `errno`, which is either
 *  - 0      No error
 *  - ERANGE Over-/underflow (set by strto* functions)
 *  - EINVAL Could not parse the desired type/string
 */
class SScan {
  const char* _s;
  int _error;
public:
  SScan(const char* s) : _s(s), _error(0) {}
  operator bool()         const noexcept { return !_error; }
  char operator[](int i)  const noexcept { return _s[i];   }
  int error()             const noexcept { return _error;  }
  const char* buffer()    const noexcept { return _s;      }
  std::size_t length()    const noexcept { return std::strlen(_s); }
  SScan& clearError()           noexcept { _error = 0; return *this; }

#if 0
static inline const char* skipWhitespace(const char *s) {
  while (*s && (*s == ' ' || *s == '\t')) { ++s; }
  return s;
}
  SScan& skip() {
  }
#endif

  SScan& read(char c) {
    if (*_s == c)
      ++_s;
    else
      _error = EINVAL;
    return *this;
  }

  template<size_t LEN>
  SScan& read(const char (&prefix)[LEN]) {
    if (! std::strncmp(_s, prefix, LEN - 1))
      _s += LEN - 1;
    else
      _error = EINVAL;
    return *this;
  }

  SScan& skip_until(const char* accept) {
    while (*_s && !std::strchr(accept, *_s))
      ++_s;
    return *this;
  }

  SScan& seek(size_t n, bool check = true) {
    if (check && n > length())
      _s += length();
    else
      _s += n;
    return *this;
  }

#define SScan_define_strto(SUFFIX, TYPE)              \
  SScan& strto ## SUFFIX (TYPE& i, int base = 10) {   \
    char *end;                                        \
    i = std::strto ## SUFFIX (_s, &end, base);        \
    if (_s == end)                                    \
      _error = EINVAL;                                \
    else if (errno)                                   \
      _error = errno;                                 \
    _s = end;                                         \
    return *this;                                     \
  }

  SScan_define_strto(imax, std::intmax_t)
  SScan_define_strto(umax, std::uintmax_t)
  SScan_define_strto(l,    long)
  SScan_define_strto(ul,   unsigned long)
  SScan_define_strto(ll,   long long)
  SScan_define_strto(ull,  unsigned long long)
#undef SScan_define_strto

  SScan& strtoi(int& i, int base = 10) {
    std::intmax_t imax;
    if (this->strtoimax(imax, base)) {
      if (imax < INT_MIN || imax > INT_MAX)
        _error = ERANGE;
      else
        i = imax;
    }
    return *this;
  }

  SScan& strtoi(short& i, int base = 10) {
    std::intmax_t imax;
    if (this->strtoimax(imax, base)) {
      if (imax < SHRT_MIN || imax > SHRT_MAX)
        _error = ERANGE;
      else
        i = imax;
    }
    return *this;
  }

#define SScan_define_strto(SUFFIX, TYPE)             \
  SScan& strto ## SUFFIX (TYPE& i) {                 \
    char *end;                                       \
    i = std::strto ## SUFFIX (_s, &end);             \
    if (_s == end)                                   \
      _error = EINVAL;                               \
    else if (errno)                                  \
      _error = errno;                                \
    _s = end;                                        \
    return *this;                                    \
  }

  SScan_define_strto(f,  float)
  SScan_define_strto(d,  double)
  SScan_define_strto(ld, long double)
#undef SScan_define_strto
};

#endif
