
#ifndef _KERNEL_UTILITY_HPP_
#define _KERNEL_UTILITY_HPP_

#include <stddef.h>


template<class T>
const T& min(const T& a, const T& b)
{
	return (b < a) ? b : a;
}
template<class T>
T& min(T& a, T& b)
{
	return (b < a) ? b : a;
}


#define has_member_function(ClassType, name, RetType, ...) \
	{ &ClassType::name } -> std::same_as<RetType(ClassType::*)(__VA_ARGS__)>


#define nop() asm("nop")


template<class T, size_t index, T First, T... Rest>
struct nth_value
	: public nth_value<T, index - 1, Rest...>
{
	static constexpr size_t args_count = 1 + sizeof...(Rest);
	static_assert(index < args_count);
};
template<class T, T Current, T... Rest>
struct nth_value<T, 0, Current, Rest...>
{
	static constexpr T value = Current;
};

template<class T, size_t N, T... Args>
static constexpr T nth_value_v = nth_value<T, N, Args...>::value;


#define ct_unreachable(msg) { []<bool always_false = false>(){ static_assert(always_false, msg); }(); }


template<class T>
consteval T ct_pow(T base, unsigned exp)
{
	T result = 1;
	while (exp > 0)
	{
		result *= base;
		--exp;
	}
	return result;
}


#endif //!_KERNEL_UTILITY_HPP_
