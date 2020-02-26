#ifndef _COMMON_HPP
#define _COMMON_HPP

#include <string>
#include <ctime>
#include <cstring>
#include <cassert>
#include <initializer_list>

#include <boost/algorithm/string.hpp>

#define BITSOF(T)     (CHAR_BIT*sizeof(T))
#define STRLEN(S)     (sizeof(S)-1)
#define ARRAY_SIZE(A) (sizeof(A)/sizeof(*A))
#define REPORT_BUG    "REPORT A BUG, PLEASE!"

#define NOT_REACHED 0
#define assert_not_reached() assert(NOT_REACHED)

/* ============================================================================
 * String functions
 * ==========================================================================*/

wchar_t* toWideString(const char* s, size_t* len = NULL);

char* toNarrowString(const wchar_t* s, size_t* len = NULL);

static inline wchar_t* toWideString(const std::string& s, size_t* len = NULL) {
  return toWideString(s.c_str(), len);
}

static inline char* toNarrowString(const std::wstring& s, size_t* len = NULL) {
  return toNarrowString(s.c_str(), len);
}

static inline char* time_format(time_t t, const char* fmt, char* buf, size_t bufsz) {
  struct tm* st = localtime(&t);
  strftime(buf, bufsz, fmt, st);
  return buf;
}

static inline const char* strMayNULL(const char* s) {
  return (s ? s : "");
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

/* ============================================================================
 * Fork()
 * ==========================================================================*/

static inline void fork_silent() {

}

static inline void open_image(const std::string& url) {
  if (fork() == 0) {
    execlp("feh", "feh", url.c_str(), NULL);
    execlp("display", "display", url.c_str(), NULL);
    execlp("xdg-open", "xdg-open", url.c_str(), NULL);
    exit(0);
  }
}

static inline void open_url(const std::string& url) {
  std::string lower = boost::algorithm::to_lower_copy(url);
  if (boost::algorithm::ends_with(lower, ".png") ||
      boost::algorithm::ends_with(lower, ".jpg") ||
      boost::algorithm::ends_with(lower, ".jpeg"))
    open_image(url);
  else
    if (fork() == 0) {
      execlp("xdg-open", "xdg-open", url.c_str(), NULL);
      exit(0);
    }
}

/* ============================================================================
 * Algorithm
 * ==========================================================================*/

template<typename T> inline bool in_list(const T &elem, const std::initializer_list<T> &list) {
  for (const auto &i : list)
    if (elem == i)
      return true;
  return false;
}

template<typename TVal, typename TLower, typename TUpper>
inline TVal clamp(TVal value, TLower lower, TUpper upper) {
  if (value < lower) return lower;
  if (value > upper) return upper;
  return value;
}

static inline size_t size_for_bits(size_t bits, size_t storage_size = 1) {
  storage_size *= CHAR_BIT;
  return (bits%storage_size ? bits/storage_size + 1 : bits/storage_size);
}

#endif
