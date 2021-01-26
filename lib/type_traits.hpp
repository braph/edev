#ifndef LIB_TYPE_TRAITS_HPP
#define LIB_TYPE_TRAITS_HPP

// Copied from:
// https://stackoverflow.com/questions/257288/templated-check-for-the-existence-of-a-class-member-function
// And it does not work. Really.

#define HAS_MEM_FUNC(func, name)                                             \
  template<typename T, typename Sign>                                        \
  struct name {                                                              \
    typedef char yes[1];                                                     \
    typedef char no [2];                                                     \
    template <typename U, U> struct type_check;                              \
    template <typename _1> static yes &chk(type_check<Sign, &_1::func > *);  \
    template <typename   > static no  &chk(...);                             \
    static bool const value = sizeof(chk<T>(0)) == sizeof(yes);              \
  }

//HAS_MEM_FUNC(c_str, has_c_str);
//HAS_MEM_FUNC(what, has_what);

#endif
