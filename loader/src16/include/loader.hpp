
#ifndef _LOADER_H_
#define _LOADER_H_

#include <stdint.h>
#include <stddef.h>

#define ptr_cast(T, ptr) ((T)(uint32_t)(ptr))

extern "C" [[noreturn]] void cpanic(const char* msg);
extern "C" void* memcpy(void* dst, const void* src, size_t bytes);

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

constexpr uint32_t KiB = 1024;
constexpr uint32_t MiB = 1024 * KiB;

#endif //!_LOADER_H_
