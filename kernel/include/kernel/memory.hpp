
#ifndef _MEMORY_HPP_
#define _MEMORY_HPP_

#include "kernel/aux/utility.hpp"

#include <stdint.h>
#include <utility>
#include <concepts>

#define MMAP_WRITABLE (uint64_t(1) << 1)
#define MMAP_USER_ACCESSABLE (uint64_t(1) << 2)
#define MMAP_WRITETHROUGH (uint64_t(1) << 3)
#define MMAP_UNCACHABLE (uint64_t(1) << 4)
#define MMAP_GLOBAL (uint64_t(1) << 8)
#define MMAP_UNEXECUTABLE (uint64_t(1) << 63)




struct mmap_result
{
	using type = uint32_t;
	enum : type
	{
		ok = 0,
		unaligned_base = 1,
		illegal_flags = 2,
		illegal_override = 3,
		insufficient_physical_pages = 4,
	};
};


/*
mmap_result::type map_virtual_range(
	uint64_t virtual_address,
	uint64_t physical_address,
	uint32_t pages,
	uint64_t flags = MMAP_WRITABLE
);
*/

mmap_result::type update_virtual_range_flags(
	uint64_t virtual_address,
	uint32_t pages,
	uint64_t flags
);



#endif //!_MEMORY_HPP_
