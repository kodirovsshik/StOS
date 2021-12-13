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
#include "disk.hpp"

#include "aux.h"
#include "mbr.h"
#include "multiloader.h"



typedef struct
{
	uint32_t partition;
	uint8_t disk = 0xFF;
	bool use_vbr : 1 = false;
} boot_entry_t;

//using partition_info_t = uint8_t;



template<class T>
struct smol_vec
{
	T* data;
	size_t size;
	size_t capacity;

	static constexpr size_t capacity_step = 8;

	void init(size_t cap)
	{
		this->data = (T*)malloc(cap * sizeof(T));
		this->size = 0;
		this->capacity = cap;
	}

	void push_back(const T& x)
	{
		if (this->size == this->capacity)
		{
			realloc(this->data, this->capacity, this->capacity + capacity_step);
			this->capacity += capacity_step;
		}
		this->data[this->size++] = x;
	}

	T& operator[](size_t i)
	{
		return this->data[i];
	}
};



smol_vec<boot_entry_t> boot_options;



void _boot(int);
int next_disk(int);
int digits_count(uint32_t);
const char* err_desc(int);



[[noreturn]]
void main()
{
	puts("StOS loader v1.0\n");
	const int nh = get_drives_count();
	const int nf = get_floppies_count();
	bool with_errors = false;

	puts("Detected ");
	put32u(nh);
	puts(" hard drive disk");
	if (nh != 1)
		putc('s');
	endl();

	if (nf)
	{
		puts("Detected ");
		put32u(nf);
		puts(" floppy disk");
		if (nf != 1)
			putc('s');
		endl();
	}

	endl();


	uint32_t vbr_partition;
	uint16_t vbr_ver = 0;
	uint8_t vbr_disk;

	disk_t* disks = (disk_t*)malloc((nf + nh) * sizeof(disk_t));
#define disk_offset(x) ((x) <= nf ? (x) : ((x) - 0x80 + nf))

	for (int i = DISK_BEGIN; i != DISK_END; i = next_disk(i))
	{
		disk_t &disk = disks[disk_offset(i)];
		int x = disk.init(i);

		if (x != 0)
			with_errors = true;

		if (x == ERR_NO_MBR || x == ERR_NO_LBA)
			continue;

		if (!disk.stos_vbr)
			continue;

		if (disk.stos_vbr_version <= vbr_ver)
			continue;

		vbr_ver = disk.stos_vbr_version;
		vbr_disk = i;
		vbr_partition = disk.stos_vbr_partition;
	}

	if (vbr_ver == 0)
	{
		puts("Warning: StOS global VBR not found in the system\n\n");
		with_errors = true;
	}



	boot_options.init(8);

	auto add_boot_option = []
	(uint8_t disk, uint32_t partition, bool use_stos_vbr)
	{
		boot_options.push_back({ partition, disk, use_stos_vbr });
	};

	for (int i = DISK_BEGIN; i != DISK_END; i = next_disk(i))
	{
		
	}

	if (boot_options.size == 0)
	{
		puts("Error: no suitable boot options found\n");
		puts("Press R to return control to BIOS or Ctrl-Alt-Del to reset\n");

		while (true)
		{
			int c = getch() & 0xFF;
			if (c == 'r' || c == 'R')
				_mbr_return();
		}
	}

	if (!with_errors && boot_options.size == 1)
	{
		puts("Single valid bootloader detected\n");
		_boot(1);
		_mbr_return();
	}

	puts("\n\nEnter 0 or nothing to return control to BIOS\nEnter bootloader number to boot: ");

	uint32_t choice = 0;
	int len = 0;
	while (true)
	{
		int pressed = getch() & 255;

		if (pressed == 13 || pressed == 10)
		{
			endl();

			if (choice == 0)
				_mbr_return();
			else
				_boot(choice);

			choice = 0;
			len = 0;
		}

		if (pressed == 8)
		{
			if (len == 0)
				continue;

			puts("\x08 \x08");
			choice /= 10;
			--len;

			continue;
		}
		else if (pressed < '0' || pressed > '9' || len == 8)
			continue;

		choice = choice * 10 + pressed - '0';

		if ((size_t)choice <= boot_options.size)
		{
			putc(pressed);
			++len;
		}
		else
			choice /= 10;
	}
}


void _boot(int)
{
}


int digits_count(uint32_t x)
{
	int d = 0;
	do
	{
		x /= 10;
		++d;
	} while (x);
	return d;
}


const char* err_desc(int n)
{
	static const char* arr[] =
	{
		"No error",
		"LBA is not supproted by disk",
		"No MBR found",
		"Partition not present",
		"(invalid error code)",
	};

	if (n < 0 || (size_t)n >= countof(arr))
		n = countof(arr) - 1;

	return arr[n];
}


int next_disk(int x)
{
	if (x >= 0 && x <= 0x7F)
	{
		if (x + 1 >= get_floppies_count())
			return 0x80;
	}

	if (x >= 0x80 && x <= 0xFF)
	{
		if (x - 0x80 + 1 >= get_drives_count())
			return DISK_END;
	}

	return x + 1;
}
