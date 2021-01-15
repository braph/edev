#ifndef LIB_STRING_HPP
#define LIB_STRING_HPP

#include <string>
#include <vector>
#include <cstring>
#include <string>
#include <ctime>
#include <cstdio>
#include <cstdarg>

// TODO: includes...
// TODO: this is a *complete* mess...

template<class T> inline T*   cstr(T* s)                          { return s;         }
template<class T> inline auto cstr(T& s) -> decltype(T{}.c_str()) { return s.c_str(); }

#if 0
template<class T, size_t N> inline size_t s_len(      T(&s)[N])   { return std::strlen(s); }
template<class T, size_t N> inline size_t s_len(const T(&s)[N])   { return N - 1;          }
template<class T>           inline size_t s_len(const T& s)       { return s.size();       }
#endif

template<class TString, class TPrefix>
inline bool starts_with(const TString& s, const TPrefix& prefix) {
  return !std::strncmp(cstr(s), cstr(prefix), std::strlen(cstr(prefix)));
}

static inline bool ends_with(const char* s, const size_t len, const char* suffix, const size_t suffix_len) {
  return len >= suffix_len && !std::strcmp(s + len - suffix_len, suffix);
}

template<class TString, class TSuffix>
inline bool ends_with(const TString& s, const TSuffix & suffix) {
  return ends_with(cstr(s), std::strlen(cstr(s)), cstr(suffix), std::strlen(cstr(suffix)));
}


static void erase_all(std::string& s, const char* search) {
  for (size_t pos; (pos = s.find(search)) != std::string::npos;)
    s.erase(pos, std::strlen(search));
}

static inline char* erase_all(char* str, char c) {
  char* it = str;
  do {
    if (*it != c)
      *str++ = *it;
  } while (*it++);
  return str;
}

static inline std::string& replace_all(std::string& s, const char* needle, size_t needle_len, char replacement) {
  for (size_t pos = 0; (pos = s.find(needle, pos, needle_len)) != std::string::npos; pos += 1)
    s.replace(pos, needle_len, 1, replacement);
  return s;
}

static inline std::string& replace_all(std::string& s, char search, const char* replacement, size_t replacement_len) {
  for (size_t pos = 0; (pos = s.find(search, pos)) != std::string::npos; pos += replacement_len)
    s.replace(pos, 1, replacement, replacement_len);
  return s;
}

static inline std::string& replace_all(std::string& s, char search, const char* replacement) {
  return replace_all(s, search, replacement, std::strlen(replacement));
}

static inline std::string& replace_all(std::string& s, const char* needle, char replacement) {
  return replace_all(s, needle, std::strlen(needle), replacement);
}

static inline bool strip_extension(std::string &s, const char* ext, size_t ext_len) {
  if (std::string::npos != s.find(ext, s.size() - ext_len)) {
    s.erase(s.size() - ext_len);
    return true;
  }
  return false;
}

static inline bool strip_extension(std::string &s, const char* ext) {
  return strip_extension(s, ext, std::strlen(ext));
}

static inline std::string& trim(std::string& s, const char* chars = " \n\t\f\v") {
  size_t count = std::strspn(s.c_str(), chars);
  if (count)
    s.erase(0, count);

  while (! s.empty() && std::strchr(chars, s.back()))
    s.pop_back();

  return s;
}

template<class String, class Predicate>
void split(std::vector<std::string>& result, const String& str, const Predicate& pred) {
  std::string rs;
  for (const char* s = cstr(str); *s; ++s) {
    if (pred(*s)) {
      result.push_back(std::move(rs));
      rs.clear();
    }
    rs.push_back(*s);
  }
  if (rs.size())
    result.push_back(std::move(rs));
}

static bool icontains(const char* haystack, const char* needle, size_t needle_len) {
  if (! needle_len)
    return true;

  while (*haystack) {
    if ((*haystack | 0x20) == (*needle | 0x20)) {
      size_t i = 1;
      for (; i < needle_len && (haystack[i] | 0x20) == (needle[i] | 0x20); ++i);
      if (i == needle_len)
        return true;
    }
    ++haystack;
  }

  return false;
}

template<class T1, class T2>
bool icontains(const T1& haystack, const T2& needle) {
  return icontains(cstr(haystack), cstr(needle), strlen(cstr(needle)));
}

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

#endif // LIB_STRING_HPP
