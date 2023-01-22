
#include <stddef.h>
#include <stdint.h>

extern "C" void* memcpy(void* dst, const void* src, size_t bytes)
{
	auto p64dst = (uint64_t*)dst;
	auto p64src = (uint64_t*)src;

	while (bytes >= 8)
	{
		*p64dst++ = *p64src++;
		bytes -= 8;
	}

	auto p8dst = (uint8_t*)p64dst;
	auto p8src = (uint8_t*)p64src;
	while (bytes)
	{
		*p8dst++ = *p8src++;
		--bytes;
	}
	return dst;
}

extern "C" void* memset(void* dst, uint8_t val, size_t bytes)
{
	const uint64_t val64 = 0x0101010101010101u * val;
	
	auto p64dst = (uint64_t*)dst;
	while (bytes >= 8)
	{
		*p64dst++ = val64;
		bytes -= 8;
	}

	auto p8dst = (uint8_t*)p64dst;
	while (bytes)
	{
		*p8dst++ = val;
		--bytes;
	}
	return dst;
	
	/*
	For some reason, if I implement this function like so:
		const uint32_t val32 = 0x01010101 * val;
		...
			*pdst++ = val32 | ((uint64_t)val32 << 32);
	I'm getting an different results on GCC depending on the compilation options
	The file is always compiled with -m16, but with -Os on top everything breaks
	If val has MSB set, the value gets miscalculated, for example val=0x80:

	x86_64-pc-elf-gcc version 13.0.0 20221220 (experimental):
		The calculated value is 0x808080808080807F
		From the assembly i can say that low dword is calculated as val32 + ((int32)val32 >> 31)
		which i do not understand at all
	
	gcc version 11.3.0 (Ubuntu 11.3.0-1ubuntu1~22.04):
		Here things get even worse
		The calculated value is 0x80808080FFFFFFFF
		which i do not understand even more

	clang version 14.0.0-1ubuntu1
	Target: x86_64-pc-none-elf
		The calculated value is 0x8080808080808080
		Works as expected, produces identical code for both implementations

	I did check, uintN_t all have required sizes and are indeed unsigned

	That is why I always use clang as my compiler
	(the binary gets heavier tho)
	*/

	static_assert(sizeof(uint64_t) == 8, "");
	static_assert(sizeof(uint32_t) == 4, "");
	static_assert(uint32_t(-1) > 0, "");

	/*
	Sanitizers come to help. The issue was that 0x01010101 * val is undefined
	for val >= 128 due to signed integer overflow
	So why again have I always thought all constants defined with 0x were unsigned?

	Still, I don't understang GCC's motivation of doing low = val32 + ((int32)val32 >> 31)
	(Especially since (int32)val32 >> 31 should always be 0 in its head, because the
	compiler assumes that no UB (that is, no signed integer overflow) will ever occur)
	Honestly, it looks more like GCC has purposely tried to screw things up in case
	overflow happens, which I would still not be happy about

	Personal conclusion: use clang and sanitizers
	*/
}
