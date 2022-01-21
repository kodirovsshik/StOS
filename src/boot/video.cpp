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



#include "interrupt.h"
#include "memory.h"





struct
{
	uint8_t *video_state_buffer = nullptr;
	uint8_t previous_video_mode = 0;
	uint8_t previous_video_page = 0;
} static bios; //all the bios-related data


struct
{
	uint16_t text_rows = 0, text_colums = 0;
} static video; //common video data



static void save_video_state()
{
	registers_info_t regs;
	regs.eax = 0x1C01;
	regs.ecx = 7;
	regs.ebx = (uint32_t)bios.video_state_buffer;
	interrupt(0x10, &regs);
}
void restore_video_state()
{
	if (bios.video_state_buffer)
	{
		registers_info_t regs;
		regs.eax = 0x1C02;
		regs.ecx = 7;
		regs.ebx = (uint32_t)bios.video_state_buffer;
		interrupt(0x10, &regs);
	}
	else
	{
		registers_info_t regs;
		regs.eax = bios.previous_video_mode;
		interrupt(0x10, &regs);

		regs.eax = 0x0500 | bios.previous_video_page;
		interrupt(0x10, &regs);
	}
}
static void try_save_video_state()
{
	registers_info_t regs;

	regs.eax = 0x0F00;
	interrupt(0x10, &regs);

	bios.previous_video_mode = regs.eax & 0xFF;
	bios.previous_video_page = (regs.ebx >> 8) & 0xFF;


	regs.eax = 0x1C00;
	regs.ecx = 7;
	regs.ebx = 0;
	interrupt(0x10, &regs);

	uint32_t buffer_size = regs.ebx * 64;

	if ((regs.eax & 0xFF) != 0x1C)
		return;

	bios.video_state_buffer = malloc_unsafe(buffer_size);
	if (!bios.video_state_buffer)
		return;

	if ((uint32_t)bios.video_state_buffer > 0xFFFF || (uint32_t)bios.video_state_buffer + buffer_size > 0xFFFF)
	{
		free(bios.video_state_buffer, buffer_size);
		return;
	}

	save_video_state();
}



void init_video()
{
	try_save_video_state();

	registers_info_t regs;
	regs.eax = 0x0003;
	interrupt(0x10, &regs);

	video.text_rows = 25;
	video.text_colums = 80;
}
