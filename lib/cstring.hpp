#ifndef LIB_CSTRING_HPP
#define LIB_CSTRING_HPP

#include <string>
#include <cstring>
#include <cwchar>
#include <ctime>
#include <cstdio>
#include <cstdarg>

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
char* time_format(char (&buf)[N], const char* fmt, std::time_t t) {
  std::tm* tm = localtime(&t);
  std::strftime(buf, N, fmt, tm);
  return buf;
}

#endif
