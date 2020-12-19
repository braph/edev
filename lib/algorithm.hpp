#ifndef LIB_ALGORITHM_HPP
#define LIB_ALGORITHM_HPP

#include <initializer_list>

template<typename T>
static inline bool in_list(const T& elem, const std::initializer_list<T>& list) {
  for (const auto &i : list)
    if (elem == i)
      return true;
  return false;
}

template<typename T>
static inline T clamp(T value, T lower, T upper) {
  if (value < lower) return lower;
  if (value > upper) return upper;
  return value;
}

template<typename T>
static inline void clamp(T* value, T lower, T upper) {
  *value = clamp(*value, lower, upper);
}

#endif
