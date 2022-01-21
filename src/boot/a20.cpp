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



#include "memory.h"
#include "bootloader.h"


#define MEGABYTE (1 << 20)
#define MEGABYTE_BIT MEGABYTE



bool need_a20 = false;
bool a20_ready = false;

void handle_a20(uint32_t begin, uint32_t end)
{
	if (a20_ready)
		return;

	if (begin & MEGABYTE_BIT)
	{
		need_a20 = true;
		if (begin + MEGABYTE >= end)
			return;
	}
	else
	{
		begin = (begin | (MEGABYTE_BIT - 1)) + 1;
		if (begin >= end)
			return;
		need_a20 = true;
	}

	volatile uint32_t *p1 = (uint32_t*)(begin);
	volatile uint32_t *p2 = (uint32_t*)(begin + MEGABYTE);

	static constexpr uint32_t c1 = 0x00FF55AA, c2 = 0xAA55FF00;

	*p1 = c1;
	*p2 = c2;

	if (*p2 - *p1 == c2 - c1)
		a20_ready = true;
}

void init_a20()
{
	if (need_a20 && !a20_ready)
		panic("Failed to activate A20 line");
}
