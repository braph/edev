#ifndef LOG_HPP
#define LOG_HPP

#include <cstdio>
#include <cstdarg>

#if defined(__GNUC__) || defined(__clang__)
  __attribute__((__format__(__printf__, 1, 2)))
#endif
static inline void log_write(const char* format, ...) noexcept {
  va_list ap;
  ::va_start(ap, format);
  ::vfprintf(stderr, format, ap);
  ::va_end(ap);
}

#endif
