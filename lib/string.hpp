#ifndef LIB_STRING_HPP
#define LIB_STRING_HPP

#include <string>
#include <vector>
#include <cstring>

template<class T> inline T*   cstr(T* s)                          { return s;         }
template<class T> inline auto cstr(T& s) -> decltype(T{}.c_str()) { return s.c_str(); }

static void erase_all(std::string& s, const char* search) {
  for (size_t pos; (pos = s.find(search)) != std::string::npos;)
    s.erase(pos, std::strlen(search));
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
