#ifndef _COMMON_HPP
#define _COMMON_HPP

#include <string>
#include <initializer_list>

#define STRLEN(S)     (sizeof(S)-1)
#define ARRAY_SIZE(A) (sizeof(A)/sizeof(*A))
#define REPORT_BUG    "REPORT A BUG"

#if DEBUG
#include <cassert>
#else
#define assert(...) (void)0
#endif

#define assert_not_reached() assert(!"reached")

wchar_t* toWideString(const char* s, size_t* len);

inline int clamp(int value, int lower, int upper) {
  if (value < lower) return lower;
  if (value > upper) return upper;
  return value;
}

template<typename T> inline bool in_list(const T &elem, const std::initializer_list<T> &list) {
  for (const auto &i : list)
    if (elem == i)
      return true;
  return false;
}

template<typename TContainer>
int proportionalGet(const TContainer &container, unsigned int size, unsigned int pos) {
  unsigned int i = container.size() * pos / size;
  return container[i];
}

template<typename TContainer>
int proportionalGet(const TContainer &container, size_t container_size, unsigned int size, unsigned int pos) {
  unsigned int i = container_size * pos / size;
  return container[i];
}

template<typename TContainer>
int proportionalGet2(const TContainer &container, unsigned int size, unsigned int pos) {
  unsigned int i = container.size() * pos * 2 / size;
  if (i >= container.size())
    i = container.size() - (i - container.size() + 1);
  return container[i];
}

template<typename TContainer>
int proportionalGet2(const TContainer &container, size_t container_size, unsigned int size, unsigned int pos) {
  unsigned int i = container_size * pos * 2 / size;
  if (i >= container_size)
    i = container_size - (i - container_size + 1);
  return container[i];
}

#if TODO_BLAH
#define LOG()
//def self.log(from, *msgs)
inline void log(const char *file, const char *func, const char *__LINE__, const std::exception &ex) {
  struct tm tm = {0};
  localtime_r(time(NULL), &tm);
  char buf[32];
  ::strftime(buf, sizeof(buf), "%Y-%m-%d 00:00:00 ", &tm);
  std::cerr << buf << from /*<< func*/ << ": " << e.what() << /*msgs.join()*/ std::endl;
}

inline void log(const char *from, const char*) {
}
#endif

#endif
