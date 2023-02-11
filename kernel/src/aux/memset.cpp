
#include <stdint.h>
#include <stddef.h>
#include "kernel/aux/utility.hpp"

#ifdef NDEBUG
#pragma GCC optimize("O3")
#endif

extern "C"
void* memset(void* ptr, int x, size_t count)
{
	auto uptr_begin = (uintptr_t)ptr;
	auto uptr_end = uptr_begin + count;

	auto uptr_unaligned = uptr_begin;
	auto uptr_aligned = (uptr_begin + 15) & ~(uintptr_t)15;
	auto uptr_remnants = uptr_end & ~(uintptr_t)15;

	const auto unaligned_count = min(uptr_unaligned - uptr_aligned, count);
	const auto uptr_unaligned_end = uptr_unaligned + unaligned_count;
	count -= unaligned_count;
	
	const auto aligned_count = count & ~(size_t)15;
	const auto uptr_aligned_end = uptr_aligned + aligned_count;
	
	const auto remnants_count = count & 15;
	const auto uptr_remnants_end = uptr_remnants + remnants_count;


	const auto x8 = (uint8_t)x;
	const auto x64 = x8 * 0x0101010101010101u;

	while (uptr_unaligned != uptr_unaligned_end)
	{
		*(uint8_t*)uptr_unaligned = x8;
		uptr_unaligned += 1;
	}
	while (uptr_aligned != uptr_aligned_end)
	{
		*(uint64_t*)(uptr_aligned + 0) = x64;
		*(uint64_t*)(uptr_aligned + 8) = x64;
		uptr_aligned += 16;
	}
	while (uptr_remnants != uptr_remnants_end)
	{
		*(uint8_t*)uptr_remnants = x8;
		uptr_remnants += 1;
	}

	return ptr;
}
