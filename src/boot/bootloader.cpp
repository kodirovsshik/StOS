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



#include "bootloader_string.h"
#include "bootloader.h"



_EXTERN_C_

const uint32_t _STACK_TOP = (uint32_t)&__STACK_TOP;

extern const char global_canary[];
extern const char default_canary[];
extern const char __canary_size[];

const size_t canary_size = (size_t)__canary_size;

_EXTERN_C_END_



void check_canary()
{
	if (memcmp(global_canary, default_canary, canary_size) != 0)
		panic("Stack smash detected");
}
