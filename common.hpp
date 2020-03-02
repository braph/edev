#ifndef _COMMON_HPP
#define _COMMON_HPP

#include <string>
#include <ctime>
#include <cstring>
#include <cassert>
#include <iostream> // XXX
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

char* toNarrowChar(wchar_t);

wchar_t* toWideString(const char* s, size_t* len = NULL);

char* toNarrowString(const wchar_t* s, size_t* len = NULL);

static inline wchar_t* toWideString(const std::string& s, size_t* len = NULL) {
  return toWideString(s.c_str(), len);
}

static inline char* toNarrowString(const std::wstring& s, size_t* len = NULL) {
  return toNarrowString(s.c_str(), len);
}

char* time_format(time_t t, const char* fmt);

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

static inline void open_image(const std::string& url) {
  if (fork() == 0) { // TODO: can be made better...
    execl("/bin/sh", "sh", "-c",
      "URL=$1; shift;"
      "for CMD; do :|$CMD \"$URL\" && break;"
      "done >/dev/null 2>/dev/null", 

      "open_image",
      // $URL
      url.c_str(),
      // $CMD's
      "feh",
      "display",
      "xdg-open",
      NULL);
    exit(0);
  }
}

static inline void open_url(const std::string& url) {
  if (boost::algorithm::iends_with(url, ".png") ||
      boost::algorithm::iends_with(url, ".jpg") ||
      boost::algorithm::iends_with(url, ".jpeg"))
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

template<typename T>
inline T clamp(T value, T lower, T upper) {
  if (value < lower) return lower;
  if (value > upper) return upper;
  return value;
}

static inline int size_for_bits(int bits, int storage_size = 1) {
  storage_size *= CHAR_BIT;
  return (bits%storage_size ? bits/storage_size + 1 : bits/storage_size);
}

static inline constexpr size_t elements_fit_in_bits(size_t bits, size_t storage_bits) {
  return (storage_bits%bits ? storage_bits/bits : storage_bits/bits);
}

#endif
