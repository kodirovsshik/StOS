
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

#endif //!_LOADER_H_
