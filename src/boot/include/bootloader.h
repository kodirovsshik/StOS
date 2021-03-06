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

#include <stdint.h>



_EXTERN_C_


extern unsigned char __STACK_TOP;
extern const uint32_t _STACK_TOP;

[[noreturn]]
void panic(const char* msg);


void check_canary();


void refresh_cmos_data();



uint8_t get_boot_disk();
uint8_t get_disks_count();
uint8_t get_floppies_count();


_EXTERN_C_END_
