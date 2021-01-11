#ifndef LIB_MATH_HPP
#define LIB_MATH_HPP

#include <type_traits>

template<class T>
typename std::enable_if<std::is_unsigned<T>::value, T>::type 
ceil_div(T divident, T divisor) {
  // Only guaranteed for unsigned types
  return divident / divisor + (divident % divisor != 0);
}

#endif
