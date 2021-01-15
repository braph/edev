#ifndef LIB_STRING_HPP
#define LIB_STRING_HPP

#include <string>
#include <vector>
#include <cstring>
#include <ctime>
#include <cstdio>
#include <cstdarg>
#include <iterator>

/**
 * Converter classes. 
 */

struct Chars {
  char *s;
  template<class T>
  inline Chars(T& s_)             noexcept : s(s_.c_str()) {}
  inline Chars(char* s_)          noexcept : s(s_)         {}
  inline Chars(signed char* s_)   noexcept : s(reinterpret_cast<char*>(s_)) {}
  inline Chars(unsigned char* s_) noexcept : s(reinterpret_cast<char*>(s_)) {}
  inline operator char*()         noexcept { return s; }
//inline char operator*()   const noexcept { return *s; }
};

struct ConstChars {
  const char *s;
  template<class T>
  inline ConstChars(const T& s_)             noexcept : s(s_.c_str()) {}
  inline ConstChars(const char* s_)          noexcept : s(s_)         {}
  inline ConstChars(const signed char* s_)   noexcept : s(reinterpret_cast<const char*>(s_)) {}
  inline ConstChars(const unsigned char* s_) noexcept : s(reinterpret_cast<const char*>(s_)) {}
  inline operator const char*()              noexcept { return s; }
//inline char operator*()   const noexcept { return *s; }
};

struct CharsLen : public Chars {
  size_t len;
  template<class T>
  inline CharsLen(T& s_)             noexcept : Chars(s_), len(std::strlen(s)) {}
  inline CharsLen(char* s_)          noexcept : Chars(s_), len(std::strlen(s)) {}
  inline CharsLen(signed char* s_)   noexcept : Chars(s_), len(std::strlen(s)) {}
  inline CharsLen(unsigned char* s_) noexcept : Chars(s_), len(std::strlen(s)) {}
};

struct ConstCharsLen : public ConstChars {
  size_t len;
  template<class T>
  inline ConstCharsLen(const T& s_)             noexcept : ConstChars(s_), len(std::strlen(s)) {}
  inline ConstCharsLen(const char* s_)          noexcept : ConstChars(s_), len(std::strlen(s)) {}
  inline ConstCharsLen(const signed char* s_)   noexcept : ConstChars(s_), len(std::strlen(s)) {}
  inline ConstCharsLen(const unsigned char* s_) noexcept : ConstChars(s_), len(std::strlen(s)) {}
};

/**
 * Predicate functions
 */

static inline bool starts_with(ConstChars s, ConstCharsLen prefix) {
  return !std::strncmp(s, prefix, prefix.len);
}

static inline bool ends_with(ConstCharsLen s, ConstCharsLen suffix) {
  return s.len >= suffix.len && !std::strcmp(s + s.len - suffix.len, suffix);
}

static bool icontains(ConstChars haystack_, ConstCharsLen needle) {
  if (! needle.len)
    return true;

  const char needle0 = *needle | 0x20;
  const char* haystack = haystack_;

  while (*haystack) {
    if ((*haystack | 0x20) == needle0) {
      size_t i = 1;
      for (; i < needle.len && (haystack[i] | 0x20) == (needle[i] | 0x20); ++i);
      if (i == needle.len)
        return true;
    }
    ++haystack;
  }

  return false;
}


#if 0

static inline size_t span(ConstChars s, ConstChars accept)  { return std::strspn(s, accept);  }
static inline size_t cspan(ConstChars s, ConstChars reject) { return std::strcspn(s, reject); }

template<class T> void shrink(T& s, size_t n) { s.resize(n); }
template<class T> void shrink(T* s, size_t n) { s[n] = '\0'; }

template<class T> size_t len(const T& s)    { return s.size(); }
inline            size_t len(const char* s) { return std::strlen(s); }

template<class T>
T& erase_all(T& s, char c) {
  auto it     = std::cbegin(s);
  auto result = std::begin(s);

  do {
    if (*it != c)
      *result++ = *it;
  } while (*it++);

  auto new_size = result - std::begin(s) - 1;
  shrink(s, new_size);
  return s;
}

template<class T>
T& trim(T& s, const char* chars = " \n\t\n\f\v") {
  size_t beg = span(s, chars);
  size_t end = len(s);
  while (end > beg && std::strchr(chars, s[--end]));
  std::memmove(&s[0], &s[beg], end - beg + 1);
  shrink(s, end - beg + 1);
  return s;
}

template<class T, class F>
void foreach_overlap(T& s) {
next:
  auto result = find(s, search);
  if (! is_npos(result)) {
    F(result);
    goto next;
  }
}

template<class T>
void eraes_all_overlap(T& s, ConstCharsLen search) {
  foreach_overlap(s, (size_t i)[]{ erase(s, i, search.len); });
}
#endif



/**
 * TODO Here begins the mess TODO
 */

template<class T> inline T*   cstr(T* s)                          { return s;         }
template<class T> inline auto cstr(T& s) -> decltype(T{}.c_str()) { return s.c_str(); }

#if 0
template<class T, size_t N> inline size_t s_len(      T(&s)[N])   { return std::strlen(s); }
template<class T, size_t N> inline size_t s_len(const T(&s)[N])   { return N - 1;          }
template<class T>           inline size_t s_len(const T& s)       { return s.size();       }
#endif

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

#if 0
#include <string>
#include <iostream>
#include <cassert>
int main(int, const char**args) {
  assert(starts_with("foo", "foo"));
  assert(starts_with("foo bar", "foo"));
  assert(ends_with("bar", "bar"));
  assert(ends_with("foo bar", "bar"));
  std::string s("foxbax"); assert(erase_all(s, 'x') == "foba");

  std::string t("  foobar  "); std::cout << '|' << trim(t) << '|' << std::endl; //assert(trim(t) == "foobar");
  std::string u(""); std::cout << '|' << trim(u) << '|' << std::endl; //assert(trim(t) == "foobar");

  //std::cout << '|' << erase_all(s, 'x') << '|' << std::endl;
  //assert(s == "foba");
}
#endif
#endif // LIB_STRING_HPP
