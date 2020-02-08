#ifndef _COMMON_HPP
#define _COMMON_HPP

#include <string>
#include <cstdio>
#include <initializer_list>

#define STRLEN(S)     (sizeof(S)-1)
#define ARRAY_SIZE(A) (sizeof(A)/sizeof(*A))

inline int clamp(int value, int lower, int upper) {
  if (value < lower) return lower;
  if (value > upper) return upper;
  return value;
}

inline std::string secondsToTime(unsigned int seconds) {
  char buf[8] = "00:00";
  if (seconds)
    sprintf(buf, "%02d:%02d", seconds/60, seconds%60);
  return buf;
}

template<typename T> inline bool in_list(const T &elem, const std::initializer_list<T> &list) {
  for (const auto &i : list)
    if (elem == i)
      return true;
  return false;
}

// TODO: better name for proportionalGet 

template<typename TContainer>
auto proportionalGet(const TContainer &container, unsigned int size, unsigned int pos) {
  unsigned int i = container.size() * pos / size;
  return container[i];
}

template<typename TContainer>
auto proportionalGet2(const TContainer &container, unsigned int size, unsigned int pos) {
  unsigned int i = container.size() * pos * 2 / size;
  if (i >= container.size())
    i = container.size() - (i - container.size() + 1);
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
