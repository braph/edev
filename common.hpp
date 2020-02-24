#ifndef _COMMON_HPP
#define _COMMON_HPP

#include <string>
#include <cstring>
#include <cassert>
#include <initializer_list>

#define STRLEN(S)     (sizeof(S)-1)
#define ARRAY_SIZE(A) (sizeof(A)/sizeof(*A))
#define REPORT_BUG    "REPORT A BUG, PLEASE!"

#define NOT_REACHED 0
#define assert_not_reached() assert(NOT_REACHED)

wchar_t* toWideString(const char* s, size_t* len = NULL);

static inline wchar_t* toWideString(const std::string& s, size_t* len = NULL) {
  return toWideString(s.c_str(), len);
}

// If `*s` starts with `prefix` advance the pointer beyond the prefix and return TRUE
template<size_t LEN>
static inline bool cstr_seek(const char **s, const char (&prefix)[LEN]) {
  if (! std::strncmp(*s, prefix, LEN - 1)) {
    *s += LEN - 1;
    return true;
  }
  return false;
}

static inline size_t size_for_bits(size_t bits, unsigned storage_size = 1) {
  storage_size *= 8 /* bits */;
  return (bits%storage_size ? bits/storage_size + 1 : bits/storage_size);
}

inline int clamp(int value, int lower, int upper) {
  if (value < lower) return lower;
  if (value > upper) return upper;
  return value;
}

template<typename T> inline bool in_list(const T &elem, const std::initializer_list<T> &list) {
  for (const auto &i : list)
    if (elem == i)
      return true;
  return false;
}

#if TODO_BLAH
#define LOG()
//def self.log(from, *msgs)
inline void log(const char *file, const char *func, const char *__LINE__, const std::exception &ex) {
  struct tm tm = {0};
  localtime_r(time(NULL), &tm);
  char buf[32];
  ::strftime(buf, sizeof(buf), "%Y-%m-%d 00:00:00 ", &tm);
  std::cerr << buf << from /*<< func*/ << ": " << e.what() << /*msgs.join()*/ std::endl;
}

inline void log(const char *from, const char*) {
}
#endif

#endif
