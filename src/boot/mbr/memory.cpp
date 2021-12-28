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



#include "io.h"
#include "aux.h"



_EXTERN_C_

uintptr_t heap_top;

void* get_heap_top()
{
	return (void*)heap_top;
}

void* malloc(size_t n)
{
	if (n & 3)
		n = (n | 3) + 1; //align up by four bytes

	static constexpr uintptr_t bound = 0x7C00;

	uintptr_t new_top = heap_top + n;
	if (new_top > bound)
	{
		puts("\nError: bootloader is out of memory");
		halt();
	}

	uintptr_t res = heap_top;
	heap_top = new_top;
	return (void*)res;
}

void realloc(void* p, size_t a, size_t b)
{
	if (a & 3)
		a = (a | 3) + 1; //align up by four bytes
	if (b & 3)
		b = (b | 3) + 1; //align up by four bytes

	if ((uintptr_t)p + a == heap_top)
		heap_top = (uintptr_t)p + b;
	else
	{
		puts("\nWarning: Reallocation failed: top = ");
		put32x((uint32_t)heap_top);

		puts(", p = ");
		put32x((uint32_t)p);

		puts(", s1 = ");
		put32x(a);

		puts(", s2 = ");
		put32x(b);

		halt();
	}
}

void free(void* p, size_t n)
{
	if (n & 3)
		n = (n | 3) + 1; //align up by four bytes

	if ((uintptr_t)p + n == heap_top)
		heap_top -= n;
	else
	{
		puts("\nWarning: Deallocation failed: top = ");
		put32x((uint32_t)heap_top);

		puts(", p = ");
		put32x((uint32_t)p);

		puts(", n = ");
		put32x(n);

		endl();
	}
}

_EXTERN_C_END_
