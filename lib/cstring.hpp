#ifndef LIB_CSTRING_HPP
#define LIB_CSTRING_HPP

#include <string>
#include <cstring>
#include <ctime>
#include <cstdio>
#include <cstdarg>

static inline const char* ensure_string(const char* s) { return (s ? s : ""); }

// ============================================================================

template<class TChar = char>
struct Basic_CString {
  Basic_CString(const Basic_CString& s) noexcept
    : _s(s._s)
    , _len(s._len)
  {}

  Basic_CString(const TChar* s) noexcept
    : _s(s)
    , _len(std::basic_string<TChar>::npos)
  {}

  template<typename T>
  Basic_CString(const TChar* s, T len) noexcept
    : _s(s)
    , _len(static_cast<size_t>(len))
  {}

  Basic_CString(const std::basic_string<TChar>& s) noexcept
    : _s(s.c_str())
    , _len(s.length())
  {}

  const TChar* c_str() const noexcept {
    return _s;
  }

  operator const TChar*() const noexcept {
    return _s;
  }

  size_t length() noexcept {
    if (_len == std::basic_string<TChar>::npos)
      _len = std::char_traits<TChar>::length(_s);
    return _len;
  }

  bool empty() noexcept {
    return length() == 0;
  }

protected:
  const TChar* _s;
  size_t _len;
};

using CString = Basic_CString<char>;

// ============================================================================

/**
 * sprintf() with temporary buffer
 */
template<size_t N>
struct temp_sprintf {

#if !defined(NDEBUG) && (defined(__GNUC__) || defined(__clang__))
  __attribute__((__format__(__printf__, 2, 3)))
  temp_sprintf(const char* fmt, ...) noexcept {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
  }
#else
  template<typename ... T>
  temp_sprintf(const char* fmt, T ... args) noexcept {
    sprintf(buf, fmt, args ...);
  }
#endif

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

#if !defined(NDEBUG) && (defined(__GNUC__) || defined(__clang__))
  __attribute__((__format__(__printf__, 2, 3)))
  temp_snprintf(const char* fmt, ...) noexcept {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, N, fmt, ap);
    va_end(ap);
  }
#else
  template<typename ... T>
  temp_snprintf(const char* fmt, T ... args) noexcept {
    snprintf(buf, N, fmt, args ...);
  }
#endif

  operator const char*() const noexcept { return buf; }

private:
  char buf[N];
};

// ============================================================================

template<size_t N>
char* time_format(char (&buf)[N], const char* fmt, std::time_t t) {
  std::tm* tm = localtime(&t);
  std::strftime(buf, N, fmt, tm);
  return buf;
}

#endif
