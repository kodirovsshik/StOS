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



#ifndef _AUX_H_
#define _AUX_H_

#include <stddef.h>
#include "defs.h"



_EXTERN_C_

void* memset  (void* dst, int val, size_t count);
void* memset32(void* dst, int val, size_t count);
void* memcpy(void* dest, const void* src, size_t count);

int strncmp(const char* s1, const char* s2, size_t max);
int memcmp (const void* p1, const void* p2, size_t size);

[[noreturn]]
void halt();

_EXTERN_C_END_



template<class T, size_t N>
consteval size_t countof(const T (&)[N])
{
	return N;
}


#endif //!_AUX_H_
