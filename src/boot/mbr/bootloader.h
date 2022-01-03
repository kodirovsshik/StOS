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



#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#include <stdint.h>
#include <stddef.h>

#include "aux.h"
#include "mbr.h"


_EXTERN_C_

[[noreturn]]
void halt();


[[noreturn]]
void __err(const char* what, const char* file, uint32_t line);
#define _err(what) __err(what, __FILE__, __LINE__)


[[noreturn]]
void _mbr_return();

[[noreturn]]
void _mbr_transfer_control_flow(const mbr_entry_t*);

//int invoke_vbr(stos_request_header_t*, uint8_t vbr_disk, uint32_t vbr_lba);


void memory_dump(const void*, size_t);


void sleep(uint32_t milliseconds);
void nanosleep(int64_t ns);



/*
extern unsigned const char __bootloader_begin;
extern unsigned const char __bootloader_end;
extern unsigned const char __bootloader_size;
extern unsigned const char STACK_TOP;
*/
extern unsigned const char MBR_VERSION;


_EXTERN_C_END_






#endif //!_BOOTLOADER_H_
