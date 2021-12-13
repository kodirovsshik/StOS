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



#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_


#include <stdint.h>

#include "bootloader.h"



typedef struct
{
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
	uint32_t eflags;
} registers_info_t;



_EXTERN_C_

void interrupt(registers_info_t*, uint8_t vector);

_EXTERN_C_END_



#endif //!_INTERRUPT_H_
