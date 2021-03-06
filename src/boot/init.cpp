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



void init_memory_allocation();
void init_a20();
void init_video();


extern bool need_a20;


_EXTERN_C_

[[noreturn]]
void invoke_main();


[[noreturn]]
void bootloader_init()
{
	init_memory_allocation();

	if (need_a20)
		init_a20();

	invoke_main();
}

_EXTERN_C_END_
