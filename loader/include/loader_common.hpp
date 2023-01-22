
#ifndef _LOADER_COMMON_H_
#define _LOADER_COMMON_H_

#define ptr_cast(T, ptr) ((T)(uint32_t)(ptr))

template<class T>
void swap(T& a, T& b)
{
	T c = b;
	b = a;
	a = c;
}
template<class T>
T min(const T& a, const T& b)
{
	return b < a ? b : a;
}

template<size_t N, class T>
constexpr size_t arr_size(const T (&)[N])
{
	return N;
}

constexpr uint32_t KiB = 1024;
constexpr uint32_t MiB = 1024 * KiB;

#endif //!_LOADER_COMMON_H_
