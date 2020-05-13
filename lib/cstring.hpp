#ifndef LIB_CSTRING_HPP
#define LIB_CSTRING_HPP

#include <string>
#include <cstring>
#include <cwchar>
#include <ctime>
#include <cstdio>
#include <cstdarg>

#define STRLEN(S) (sizeof(S)-1)

static inline const char* ensure_string(const char* s) { return (s ? s : ""); }

// ============================================================================

struct CString {
  CString(const CString& s) noexcept
    : _s(s._s)
    , _len(s._len)
  {}

  CString(const char* s) noexcept
    : _s(s)
    , _len(std::string::npos)
  {}

  template<typename T>
  CString(const char* s, T len) noexcept
    : _s(s)
    , _len(static_cast<size_t>(len))
  {}

  CString(const std::string& s) noexcept
    : _s(s.c_str())
    , _len(s.length())
  {}

  const char* c_str() const noexcept {
    return _s;
  }

  operator const char*() const noexcept {
    return _s;
  }

  size_t length() noexcept {
    if (_len == std::string::npos)
      _len = std::strlen(_s);
    return _len;
  }

  bool empty() noexcept {
    return !_len;
  }

protected:
  const char* _s;
  size_t _len;
};

// ============================================================================

struct CWString {
  CWString(const CWString& s) noexcept
    : _s(s._s)
    , _len(s._len)
  {}

  CWString(const wchar_t* s) noexcept
    : _s(s)
    , _len(std::wstring::npos)
  {}

  template<typename T>
  CWString(const wchar_t* s, T len) noexcept
    : _s(s)
    , _len(static_cast<size_t>(len))
  {}

  CWString(const std::wstring& s) noexcept
    : _s(s.c_str())
    , _len(s.length())
  {}

  const wchar_t* c_str() const noexcept {
    return _s;
  }

  operator const wchar_t*() const noexcept {
    return _s;
  }

  size_t length() noexcept {
    if (_len == std::wstring::npos)
      _len = std::wcslen(_s);
    return _len;
  }

  bool empty() noexcept {
    return !_len;
  }

protected:
  const wchar_t* _s;
  size_t _len;
};

// ============================================================================

/**
 * sprintf() with temporary buffer
 */
template<size_t N>
struct temp_sprintf {

#if defined(__GNUC__) || defined(__clang__)
  __attribute__((__format__(__printf__, 2, 3)))
#endif
  temp_sprintf(const char* fmt, ...) noexcept {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
  }

  operator const char*() const noexcept { return buf; }

private:
  char buf[N];
};

// ============================================================================

/**
 * snprintf() with temporary buffer
 */
template<size_t N>
struct temp_snprintf {

#if defined(__GNUC__) || defined(__clang__)
  __attribute__((__format__(__printf__, 2, 3)))
#endif
  temp_snprintf(const char* fmt, ...) noexcept {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, N, fmt, ap);
    va_end(ap);
  }

  operator const char*() const noexcept { return buf; }

private:
  char buf[N];
};

// ============================================================================

template<size_t N>
inline bool strprefix(const char* s, const char (&prefix)[N]) {
  return !std::strncmp(s, prefix, N-1);
}

static bool ends_with(const char* s, const size_t len, const char* suffix, const size_t suffix_len) {
  return len >= suffix_len && !std::strcmp(s + len - suffix_len, suffix);
}

template<size_t N>
inline bool ends_with(const std::string& s, const char (&prefix)[N]) {
  return ends_with(&s[0], s.size(), prefix, N - 1);
}

static inline char* erase_all(char* str, char c) {
  char* it = str;
  do {
    if (*it != c)
      *str++ = *it;
  } while (*it++);
  return str;
}

static inline std::string& trim(std::string& s, const char* chars = " \n\t\f\v") {
  size_t count = std::strspn(s.c_str(), chars);
  if (count)
    s.erase(0, count);

  while (! s.empty() && std::strchr(chars, s.back()))
    s.pop_back();

  return s;
}

#if 0
void split(std::vector<std::string>& r, const std::string& s, const char c) {
  r.clear();
}
#endif

char*    toNarrowChar(wchar_t);
wchar_t* toWideString(CString, size_t* len = NULL);
char*    toNarrowString(CWString, size_t* len = NULL);
char*    time_format(std::time_t, CString);

#endif
