#ifndef LIB_TMP_ARRAY_GENERATOR_HPP
#define LIB_TMP_ARRAY_GENERATOR_HPP

namespace tmp_array_generator_impl {

template <typename T, T ...Args>
struct Data
{
	static T value[sizeof...(Args)];
};

template <typename T, T ...Args>
T Data<T, Args...>::value[sizeof...(Args)] = { Args... };


template<typename T, size_t N, template <size_t I> class Callback, T ...Args>
struct Generator
{
	using data = typename Generator<T, N - 1, Callback, Callback<N - 1>::value, Args...>::data;
};

template<typename T, template <size_t I> class Callback, T ...Args>
struct Generator<T, 1, Callback, Args...>
{
	using data = Data<T, Callback<1>::value, Args...>;
};

} // namespace tmp_array_generator_impl

template<typename T, size_t N, template <size_t I> class Callback>
struct generate_array
{
	using data = typename tmp_array_generator_impl::Generator<T, N, Callback>::data;
};

#endif
