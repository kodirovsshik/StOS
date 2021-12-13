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
#include "bootloader.h"
#include "interrupt.h"



_EXTERN_C_


const static char digits[] = "0123456789ABCDEF";

void put0x32x(uint32_t x)
{
	puts("0x");
	put32x(x);
}
void put32x(uint32_t x)
{
	char buffer[9];
	char* p = buffer + 8;
	*p = 0;

	for (int i = 8; i > 0; --i)
	{
		*--p = digits[x & 15];
		x >>= 4;
	}

	puts(p);
}
void put32u(uint32_t u)
{
	char buffer[16];
	char* p = buffer + 15;
	*p = 0;

	do
	{
		*--p = '0' + (u % 10);
		u /= 10;
	} while (u);

	puts(p);
}

void memory_dump(const void* _p, size_t n)
{
	const uint8_t* p = (uint8_t*)_p;

	while (n --> 0)
	{
		uint8_t x = *p++;
		putc(digits[x >> 4]);
		putc(digits[x & 15]);
	}
}



uint8_t current_boot_drive;

uint8_t get_boot_drive()
{
	return current_boot_drive;
}
uint8_t get_floppies_count()
{
	for (int i = 0; i < 0x80; ++i)
	{
		registers_info_t regs;
		regs.eax = 0x0800;
		regs.edx = i;

		interrupt(&regs, 0x13);

		if (regs.eflags & 1)
			return i;
	}
	return 0x80;
}



uint8_t _drive_lba_helper(void*, uint8_t);

typedef struct
{
	uint8_t size = 16;
	uint8_t command;
	uint16_t n;
	uint16_t dst_offset;
	uint16_t dst_segment;
	uint64_t first;
} __attribute__((packed)) lba_packet16_t;

#define LBA_COMMAND_READ 0x42
#define LBA_COMMAND_WRITE 0x43

uint8_t drive_lba_command(uint8_t disk, uint32_t lba, void* ptr, uint16_t num, uint8_t command)
{
	lba_packet16_t packet;
	packet.first = lba;
	packet.command = command;

	uintptr_t p = (uintptr_t)ptr;
	while (num > 0)
	{
		uint16_t current = (num > 64) ? 64 : num;

		packet.dst_offset = p & 0xF;
		packet.dst_segment = p >> 4;
		packet.n = current;

		uint8_t status = _drive_lba_helper(&packet, disk);
		//memory_dump(&packet, sizeof(packet)); endl();
		if (status)
			return status;

		num -= current;
		p += 512 * current;
		packet.first += current;
	}

	return 0;
}

uint8_t write_drive_lba(uint8_t disk, uint32_t lba, const void* ptr, uint16_t num)
{
	return drive_lba_command(disk, lba, (void*)ptr, num, LBA_COMMAND_WRITE);
}
uint8_t read_drive_lba(uint8_t disk, uint32_t lba,        void* ptr, uint16_t num)
{
	return drive_lba_command(disk, lba, (void*)ptr, num, LBA_COMMAND_READ);
}

void tabulate(int n)
{
	for (int i = 0; i < n; ++i)
		putc(' ');
}


_EXTERN_C_END_
