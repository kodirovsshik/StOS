
#ifndef _RAM_HPP_
#define _RAM_HPP_

struct memory_map_entry_t
{
	uint64_t begin, end;
	uint32_t type;
	uint32_t flags;
};
static_assert(sizeof(memory_map_entry_t) == 24, "");

#endif //!_RAM_HPP_
