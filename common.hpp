#ifndef COMMON_HPP
#define COMMON_HPP

#include <string>
#include <ctime>
#include <cassert>
#include <initializer_list>

#include <boost/algorithm/string/predicate.hpp>

#define BITSOF(T)     (CHAR_BIT*sizeof(T))
#define STRLEN(S)     (sizeof(S)-1)
#define ARRAY_SIZE(A) (sizeof(A)/sizeof(*A))
#define REPORT_BUG    "REPORT A BUG, PLEASE!"

#define NOT_REACHED 0
#define assert_not_reached() assert(NOT_REACHED)

/* ============================================================================
 * String functions
 * ==========================================================================*/

struct CString {
  CString(const CString& s)     noexcept : _s(s._s) {}
  CString(const char* s)        noexcept : _s(s) {}
  CString(const std::string& s) noexcept : _s(s.c_str()) {}
  operator const char*() const  noexcept { return _s; }
private:
  const char* _s;
};

struct CWString {
  CWString(const CWString& s)     noexcept : _s(s._s) {}
  CWString(const wchar_t* s)      noexcept : _s(s) {}
  CWString(const std::wstring& s) noexcept : _s(s.c_str()) {}
  operator const wchar_t*() const noexcept { return _s; }
private:
  const wchar_t* _s;
};

char*    toNarrowChar(wchar_t);
wchar_t* toWideString(CString, size_t* len = NULL);
char*    toNarrowString(CWString, size_t* len = NULL);
char*    time_format(time_t, CString);

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
  if (boost::algorithm::iends_with(url, ".png")
      || boost::algorithm::iends_with(url, ".jpg")
      || boost::algorithm::iends_with(url, ".jpeg"))
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

template<typename T>
static inline bool in_list(const T &elem, const std::initializer_list<T> &list) {
  for (const auto &i : list)
    if (elem == i)
      return true;
  return false;
}

template<typename T>
static inline T clamp(T value, T lower, T upper) {
  if (value < lower) return lower;
  if (value > upper) return upper;
  return value;
}

static inline size_t size_for_bits(size_t bits, size_t storage_size = 1) {
  storage_size *= CHAR_BIT;
  return (bits%storage_size ? bits/storage_size + 1 : bits/storage_size);
}

static inline constexpr size_t elements_fit_in_bits(size_t bits, size_t storage_bits) {
  return (storage_bits%bits ? storage_bits/bits : storage_bits/bits);
}

#endif
