
#include <stddef.h>
#include <stdint.h>

extern "C" void* memcpy(void* dst, const void* src, size_t bytes)
{
	auto pdst = (uint64_t*)dst;
	auto psrc = (uint64_t*)src;

	while (bytes >= 8)
	{
		*pdst++ = *psrc++;
		bytes -= 8;
	}

	auto p8dst = (uint8_t*)pdst;
	auto p8src = (uint8_t*)psrc;
	while (bytes)
	{
		*p8dst++ = *p8src++;
		--bytes;
	}
	return dst;
}
