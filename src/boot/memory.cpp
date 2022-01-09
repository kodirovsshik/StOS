#if 0
	  This file is a part of StOS project - a small operating system
	  	made for learning purposes
	  Copyright (C) 2021 Egorov Stanislav, kodirovsshik@mail.ru, kodirovsshik@gmail.com

	  This program is free software: you can redistribute it and/or modify
	  it under the terms of the GNU General Public License as published by
	  the Free Software Foundation, either version 3 of the License, or
	  (at your option) any later version.

	  This program is distributed in the hope that it will be useful,
	  but WITHOUT ANY WARRANTY; without even the implied warranty of
	  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	  GNU General Public License for more details.

	  You should have received a copy of the GNU General Public License
	  along with this program.  If not, see <https://www.gnu.org/licenses/>.
#endif



#include "defs.h"
#include "bootloader.h"
#include "interrupt.h"
#include "bootloader_string.h"
#include "aux.h"

#include <stdint.h>
#include <stddef.h>



struct memory_map_entry_t
{
	uint64_t begin;
	uint64_t end;
};

class memory_map_iterator_t
{
	struct bios_memory_map_entry_t
	{
		uint64_t begin;
		uint64_t size;
		uint32_t type;
		uint32_t flags;
	} bios_memory_map_entry;

	static_assert(sizeof(bios_memory_map_entry) == 24, "");


	memory_map_entry_t memory_map_entry;

	uint32_t index = -1;
	bool last = false;



	void invalidate()
	{
		this->index = -1;
	}

	void set_last()
	{
		this->last = true;
	}


public:

	static bool available()
	{
		bios_memory_map_entry_t entry;

		registers_info_t regs;
		regs.eax = 0x0000E820;
		regs.edx = 0x534D4150;
		regs.ecx = sizeof(entry);
		regs.ebx = 0x00000000;
		regs.edi = (uint16_t)(uintptr_t)&entry;

		interrupt(0x15, &regs);

		return regs.eax == 0x534D4150 && !(regs.eflags & EFLAGS_CF);
	}



	void start()
	{
		this->last = false;
		this->index = 0;
		this->next();
	}

	explicit operator bool() const
	{
		return this->index != (uint32_t)-1;
	}

	void next(uint32_t min_block_size = 64)
	{
		if (this->index == (uint32_t)-1)
			return;

		do
		{
			if (this->last)
			{
				this->invalidate();
				break;
			}

			this->bios_memory_map_entry.flags = 1;

			registers_info_t regs;
			regs.eax = 0x0000E820;
			regs.edx = 0x534D4150;
			regs.ecx = sizeof(this->bios_memory_map_entry);
			regs.ebx = this->index;
			regs.edi = (uint16_t)(uintptr_t)&this->bios_memory_map_entry;

			interrupt(0x15, &regs);

			this->index++;

			if (regs.eax != 0x534D4150)
			{
				this->invalidate();
				break;
			}

			if (regs.ebx == 0 || (regs.eflags & EFLAGS_CF))
				this->set_last();

			if (this->bios_memory_map_entry.type != 1)
				continue;

			if (this->bios_memory_map_entry.begin & 0xFFFFFFFF00000000)
				continue;

			if (regs.ecx >= 24 && (this->bios_memory_map_entry.flags & 3) != 1)
				continue;

			if (this->bios_memory_map_entry.size < min_block_size)
				continue;

			break;

		} while (true);

		this->memory_map_entry.begin = this->memory_map_entry.end = 0;

		if (*this)
		{
			this->memory_map_entry.begin =
				this->bios_memory_map_entry.begin;
			this->memory_map_entry.end =
				this->bios_memory_map_entry.begin + this->bios_memory_map_entry.size;

			if (this->memory_map_entry.begin & 3)
				this->memory_map_entry.begin = (memory_map_entry.begin | 3) + 1;
			this->memory_map_entry.end &= ~3;
		}
	}

	const memory_map_entry_t* operator->() const
	{
		return &this->memory_map_entry;
	}

	const memory_map_entry_t& operator*() const
	{
		return this->memory_map_entry;
	}

	void store(memory_map_entry_t* p) const
	{
		memcpy(p, &this->memory_map_entry, sizeof(this->memory_map_entry));
	}
};





struct memory_block_descriptor_t
{
	uint32_t begin;
	uint32_t end;
	uint32_t top;
	memory_block_descriptor_t *next, *prev;
};

static_assert( sizeof(memory_block_descriptor_t) == 20, "");
static_assert(alignof(memory_block_descriptor_t) == 4 , "");



memory_block_descriptor_t* memory_blocks_list = nullptr;
memory_block_descriptor_t* memory_blocks_end = nullptr;



void init_memory_block(uint32_t begin, uint32_t end)
{
	memory_block_descriptor_t* block;

	uint32_t top;
	if (memory_blocks_list == nullptr) [[unlikely]]
	{
		if (begin != 0)
			panic("Memory initialization failed: First block does not start at 0");

		begin += _STACK_TOP + sizeof(memory_block_descriptor_t);
		top = begin;

		if (begin >= end)
			panic("Memory initialization failed: Not enough low memory");

		block = (memory_block_descriptor_t*)_STACK_TOP;
		memory_blocks_list = block;
	}
	else [[likely]]
	{
		//Find some space in a first available block
		memory_block_descriptor_t* current_block = memory_blocks_list;
		while (current_block)
		{
			uint32_t available = current_block->end - current_block->top;
			if (available >= sizeof(memory_block_descriptor_t))
			{
				block = (memory_block_descriptor_t*)current_block->top;
				current_block->top += sizeof(memory_block_descriptor_t);
				break;
			}
			current_block = current_block->next;
		}

		if (current_block == nullptr)
			panic("Memory initialization failed: Failed to allocate memory for memory blocks");

		top = begin;
	}

	block->begin = begin;
	block->top = top;
	block->end = end;
	block->next = nullptr;
	block->prev = memory_blocks_end;
	if (block->prev)
		block->prev->next = block;

	memory_blocks_end = block;
}

bool init_memory_allocation_smol()
{
	registers_info_t regs;
	regs.eax = 0;
	interrupt(0x12, &regs);

	init_memory_block(0, regs.eax * 1024);

	return true;
}

bool init_memory_allocation_e820()
{
	if (!memory_map_iterator_t::available())
		return false;

	memory_map_entry_t entry;
	memory_map_iterator_t iter;
	for (iter.start(); iter; iter.next())
	{
		iter.store(&entry);
		init_memory_block(entry.begin, entry.end);
	}

	return true;
}



void init_memory_allocation()
{
	if (init_memory_allocation_e820())
		return;
	if (init_memory_allocation_smol())
		return;

	panic("Failed to initialize memory allocation");
}




size_t align_up_allocation_size(size_t x)
{
	if (x & 3)
		x = (x | 3) + 1;
	return x;
}

uint8_t* malloc(size_t n)
{
	n = align_up_allocation_size(n);

	memory_block_descriptor_t* current_block = memory_blocks_list;
	while (current_block)
	{
		uint32_t available = current_block->end - current_block->top;
		if (available >= n)
		{
			memory_blocks_list = current_block;
			current_block->top += n;
			return (uint8_t*)(current_block->top - n);
		}
		current_block = current_block->next;
	}

	panic("Memory allocation failed");
}
void free(void* p, size_t n)
{
	n = align_up_allocation_size(n);
	memory_block_descriptor_t* current_block = memory_blocks_list;
	uint32_t pu = (uint32_t)p;

	while (current_block != nullptr)
	{
		if (pu < current_block->begin && pu >= current_block->top)
			current_block = current_block->prev;
		else
		{
			if (pu + n == current_block->top)
			{
				memory_blocks_list = current_block;
				current_block->top -= n;
			}
			break;
		}
	}
}

