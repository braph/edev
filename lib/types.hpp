#ifndef LIB_TYPES_HPP
#define LIB_TYPES_HPP

#include <type_traits>

template<class T>
struct any_sign {
  T i;
  using Signed = typename std::make_signed<T>::type;
  using Unsign = typename std::make_unsigned<T>::type;
  any_sign(Signed i_) : i(static_cast<T>(i_)) {}
  any_sign(Unsign i_) : i(static_cast<T>(i_)) {}
  operator Signed() const noexcept { return static_cast<Signed>(i); }
  operator Unsign() const noexcept { return static_cast<Unsign>(i); }
};

template<class T>
any_sign<T> auto_sign(T i) { return any_sign<T>(i); }

template<class TSign, class T>
using sign_cast_result_t =
  typename std::conditional< std::is_same<TSign, signed>::value,   typename std::make_signed<T>::type,
  typename std::conditional< std::is_same<TSign, unsigned>::value, typename std::make_unsigned<T>::type, void>::type>::type;

template<class TSign, class T>
auto sign_cast(T v) -> sign_cast_result_t<TSign, T>
{ return static_cast<sign_cast_result_t<TSign, T>>(v); }

#endif
