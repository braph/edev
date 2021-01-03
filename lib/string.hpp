#ifndef LIB_STRING_HPP
#define LIB_STRING_HPP

#include <string>
#include <vector>
#include <cstring>

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

static bool ends_with(const char* s, const size_t len, const char* suffix, const size_t suffix_len) {
  return len >= suffix_len && !std::strcmp(s + len - suffix_len, suffix);
}

template<size_t N>
inline bool ends_with(const std::string& s, const char (&prefix)[N]) {
  return ends_with(&s[0], s.size(), prefix, N - 1);
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

static bool icontains(const char* haystack, const char* needle) {
  if (!*needle)
    return true;

  const char *h, *n;

  while (*haystack) {
    if ((*haystack | 0x20) == (*needle| 0x20)) {
      h = haystack + 1;
      n = needle + 1;
      for (; *n && *h; ++h, ++n)
        if ((*h | 0x20) != (*n | 0x20))
          goto NEXT;
      if (*n == *h)
        return true;
    }
NEXT:
    ++haystack;
  }

  return false;
}

template<class T1, class T2>
bool icontains(const T1& haystack, const T2& needle) {
  return icontains(cstr(haystack), cstr(needle));
}

#endif // LIB_STRING_HPP
