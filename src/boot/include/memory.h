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



#include <stddef.h>
#include <stdint.h>



#ifndef _MEMORY_H_
#define _MEMORY_H_


struct memory_block_descriptor_t
{
	uint32_t begin;
	uint32_t end;
	uint32_t top;
	memory_block_descriptor_t *next, *prev;
};

memory_block_descriptor_t* get_heap_state();

uint8_t* malloc(size_t);
uint8_t* malloc_unsafe(size_t);
void free(void*, size_t);


#endif //!_MEMORY_H_
