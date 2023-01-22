
#ifndef _LOADER_H_
#define _LOADER_H_

#include <stdint.h>
#include <stddef.h>

extern "C" [[noreturn]] void cpanic(const char* msg);
extern "C" void* memcpy(void* dst, const void* src, size_t bytes);


#endif //!_LOADER_H_
