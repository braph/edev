#ifndef LIB_STRING_HPP
#define LIB_STRING_HPP

#include <string>
#include <vector>
#include <cstring>

// TODO...

template<class T> inline T*   cstr(T* s)                          { return s;         }
template<class T> inline auto cstr(T& s) -> decltype(T{}.c_str()) { return s.c_str(); }

static void erase_all(std::string& s, const char* search) {
  for (size_t pos; (pos = s.find(search)) != std::string::npos;)
    s.erase(pos, std::strlen(search));
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

template<class T1, class T2>
bool icontains(const T1& haystack, const T2& needle) {
  return icontains(cstr(haystack), cstr(needle));
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

#endif // LIB_STRING_HPP
