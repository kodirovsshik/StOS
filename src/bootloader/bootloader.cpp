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
#include "disk.hpp"
#include "aux.h"



_EXTERN_C_


[[noreturn]]
void __err(const char* what, const char* file, uint32_t line)
{
	puts("\n\nCRITICAL ERROR:\n");
	puts(what);
	puts("\n\nAt \"");
	puts(file);
	puts("\":");
	put32u(line);
	puts("\nEXECUTION HALTED");
	halt();
}



#if 0
//void _sleep_ticks(uint16_t ticks);
void _sleep_ns_unchecked(int ns);

void nanosleep(int64_t ns)
{
	static constexpr int threshold = 54923771;

	while (ns > 0)
	{
		uint64_t current = (ns > threshold) ? threshold : ns;

		_sleep_ns_unchecked(current);
		ns -= current;
	}
}

void sleep(uint32_t ms)
{
	nanosleep((uint64_t)ms * 1000000);
}
#endif


uintptr_t heap_top = (uintptr_t)&STACK_TOP;

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



int _invoke_vbr(stos_request_header_t*, const void*, uint8_t);

int invoke_vbr(stos_request_header_t* hdr, uint8_t vbr_disk, uint32_t vbr_lba)
{
	if (read_drive_lba(vbr_disk, vbr_lba, (void*)0x7C00, 1) != 0)
		return STOS_REQ_INVOKE_ERR_READ_ERR;

	return _invoke_vbr(hdr, "StOSrequ", vbr_disk);
}

_EXTERN_C_END_
