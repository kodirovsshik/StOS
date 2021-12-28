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
#include "memory.h"
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

	void clear()
	{
		this->size = 0;
	}
	void deallocate()
	{
		free(this->data, this->capacity);
		this->size = this->capacity = 0;
	}
};



smol_vec<boot_entry_t> boot_options;



extern "C" uint32_t _invoke_vbr_helper(stos_request_header_t*, uint8_t disk, mbr_entry_t*);
uint32_t _boot(uint32_t n, uint8_t vbr_disk, uint32_t vbr_partition, uint8_t* read_status);
int next_disk(int);
int digits_count(uint32_t);
const char* err_desc(int);
uint32_t invoke_vbr(uint8_t disk, uint32_t partition, stos_request_header_t*, uint8_t* read_status);
bool is_non_fatal_floppy(uint8_t disk, uint8_t status);



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
		uint8_t x = disk.init(i);

		if (x != 0 && !is_non_fatal_floppy(i, x))
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

	auto print_partition_info = []
	(uint8_t disk, uint32_t partition, uint32_t boot_number, uint8_t tabulation = 0)
	{
		if (boot_number)
		{
			tabulate(tabulation);
			putc('[');
			put32u(boot_number);
			puts("] ");
		}
		else
			tabulate(tabulation + 3 + digits_count(boot_number));

		if (disk >= 0x80)
		{
			puts("Disk ");
			put32u(disk - 0x80 + 1);
		}
		else
		{
			puts("Floppy ");
			put32u(disk + 1);
		}

		if (partition)
		{
			puts(" Partition ");
			put32u(partition);
		}
		else if (disk == get_boot_drive())
			puts(" (current)");
	};

	auto add_boot_option = [&]
	(uint8_t disk, uint32_t partition, bool use_stos_vbr)
	{
		boot_options.push_back({ partition, disk, use_stos_vbr });
		print_partition_info(disk, partition, boot_options.size);
		endl();
	};

	auto process_error = [&]
	(const char* msg, uint32_t err_code, uint8_t read_err_code)
	{
		puts(msg);

		puts((err_code & 0x80000000) ? get_vbr_invoke_error_desc(err_code) : err_desc(err_code));
		if (err_code == ERR_READ)
		{
			puts(", error code ");
			if (read_err_code != 0xFE)
				put32u(read_err_code);
			else
				puts("not available");
			endl();
		}
		with_errors = true;
	};

	for (int i = DISK_BEGIN; i != DISK_END; i = next_disk(i))
	{
		disk_t disk;
		disk.init(i);

		if (disk.init_status == 0 && disk.has_mbr && disk.bios_number != get_boot_drive())
			add_boot_option(i, 0, true);
		else if (disk.init_status == 0)
		{
			print_partition_info(i, 0, false);
			endl();
		}

		if (disk.init_status != 0)
		{
			print_partition_info(i, 0, false);
			if (!is_non_fatal_floppy(i, disk.init_status))
			{
				puts(": ");
				puts(err_desc(disk.init_status));
				with_errors = true;
			}
			endl();
			continue;
		}

		stos_boot_list_t* list = nullptr;
		uint32_t list_top = 0;

		if (vbr_ver != 0)
		{
			stos_request_header_t header;
			header.struct_size = sizeof(header);
			header.request_number = STOS_REQ_GET_MBR_BOOT_OPTIONS;
			header.request_data = i;
			header.mbr_version = (uint16_t)(uintptr_t)&MBR_VERSION;
			header.free_memory_begin = (uint16_t)(uintptr_t)get_heap_top();

			uint8_t x8 = 0xFE;
			uint32_t x = invoke_vbr(vbr_disk, vbr_partition, &header, &x8);
			if (x != STOS_REQ_INVOKE_SUCCESS)
			{
				process_error("Subroutine invoke error: ", x, x8);
			}
			else
			{
				list = (stos_boot_list_t*)(uintptr_t)header.request_data;
			}
		}

		for (auto it = disk.begin(); it.valid(); it.next())
		{
			mbr_partition_info_t info;
			uint32_t x = it.get_mbr_info(&info);
			if (x == ERR_NO_PARTITION)
				continue;
			if (x != ERR_WRONG_PARTITION_TABLE_TYPE)
			{
				print_partition_info(disk.bios_number, it.partition_number + 1, 0);
				puts(": ");
				puts(err_desc(x));
				endl();
				continue;
			}

			stos_boot_list_t::entry_t* current = list ? &list->arr[list_top] : nullptr;
			bool in_list = (list && current->n == it.partition_number);

			if (info.active || (list && current->status == bootability_status_t::bootable))
				add_boot_option(i, it.partition_number, in_list);
			else
			{
				print_partition_info(disk.bios_number, it.partition_number + 1, 0);
				endl();
			}

			if (in_list && ++list_top == list->size)
			{
				list_top = 0;
				list = list->next;
			}
		}
	}

	if (!with_errors && boot_options.size == 1)
	{
		puts("Single valid bootloader detected\n");
		uint8_t rd = 0xFE;
		uint32_t x;
		x = _boot(1, vbr_disk, vbr_partition, &rd);
		process_error("Boot error: ", x, rd);
		boot_options.clear();
	}

	if (boot_options.size == 0)
	{
		puts("Error: no suitable boot options found\n");
		puts("Press R to transfer control to BIOS or Ctrl-Alt-Del to reset\n");

		while (true)
		{
			int c = getch() & 0xFF;
			if (c == 'r' || c == 'R')
				_mbr_return();
		}
	}

	while (true)
	{
		puts("\n\nEnter 0 or nothing to return control to BIOS\nEnter bootloader number to boot: ");

		uint32_t x;
		if (!get32u(&x, boot_options.size) || x == 0)
			_mbr_return();

		uint8_t x8;
		x = _boot(x, vbr_disk, vbr_partition, &x8);
		process_error("Boot error: ", x, x8);
	}
}



uint32_t invoke_vbr(uint8_t disk_n, uint32_t partition, stos_request_header_t* hdr, uint8_t* read_status)
{
	disk_t disk;
	if (disk.init(disk_n) != 0)
		return disk.init_status;

	uint64_t lba;
	uint32_t x;

	auto it = disk.begin();
	it.partition_number = partition;

	if (disk.has_gpt)
	{
		{
			gpt_partition_info_t info;
			x = it.get_gpt_info(&info);
			if (read_status) *read_status = disk.read_status;
			if (x) return x;
			lba = info.first;
		}

		{
			mbr_bootloader_t mbr;
			x = read_drive_lba(disk_n, 0, &mbr, 1);
			if (x) return ERR_READ;
			if (read_status) *read_status = disk.read_status;
			if (mbr.signature != 0xAA55 || strncmp(mbr.metadata.signature, "StOSboot", 8) != 0)
				return ERR_NO_MBR;
			lba += mbr.metadata.size - 1;
		}
	}
	else
	{
		mbr_partition_info_t info;
		x = it.get_mbr_info(&info);
		if (read_status) *read_status = disk.read_status;
		if (x) return x;
		lba = info.lba;
	}

	x = read_drive_lba(disk_n, lba, (void*)0x7C00, 2);
	if (x && read_status) *read_status = x;
	if (x)
		return ERR_READ;

	mbr_entry_t entry;
	entry.active = disk_n;
	entry.start_lba = (uint32_t)lba;
	entry.count_lba = (uint32_t)(lba >> 32);

	return _invoke_vbr_helper(hdr, disk_n, &entry);
}

uint32_t _boot(uint32_t n, uint8_t vbr_disk, uint32_t vbr_partition, uint8_t* read_status)
{
	auto& entry = boot_options[n - 1];

	//aim's partition
	uint32_t aimp = entry.use_vbr ? vbr_partition : entry.partition;
	uint8_t aimd = entry.use_vbr ? vbr_disk : entry.disk;
	//aim's disk

	disk_t disk;
	disk.init(aimd);
	if (disk.init_status != 0)
	{
		if (read_status) *read_status = disk.read_status;
		return disk.init_status;
	}

	auto iter = disk.begin();
	iter.partition_number = aimp;

	uint64_t aiml; //aim's lba
	uint32_t x;
	if (disk.has_gpt)
	{
		gpt_partition_info_t info;
		x = iter.get_gpt_info(&info);
		if (read_status) *read_status = disk.read_status;
		if (x) return x;
		aiml = info.first;
	}
	else
	{
		mbr_partition_info_t info;
		x = iter.get_mbr_info(&info);
		if (read_status) *read_status = disk.read_status;
		if (x) return x;
		aiml = info.lba;
	}

	if (!entry.use_vbr)
	{
		x = read_drive_lba(entry.disk, aiml, (void*)0x7C00, 2);
		if (x && read_status) *read_status = x;
		if (x) return ERR_READ;

		if (disk.has_gpt)
			return ERR_WRONG_PARTITION_TABLE_TYPE;

		mbr_entry_t e;
		x = iter.get_mbr_entry(&e);
		if (read_status) *read_status = disk.read_status;
		if (x) return x;
		_mbr_transfer_control_flow(&e);
	}
	else
	{
		stos_boot_req_data_t req_data;
		req_data.partition = entry.partition;
		req_data.disk = entry.disk;
		req_data.is_gpt = disk.has_gpt;

		stos_request_header_t hdr;
		hdr.struct_size = sizeof(hdr);
		hdr.request_number = STOS_REQ_BOOT;
		hdr.request_data = (uint16_t)(uintptr_t)&req_data;
		return invoke_vbr(vbr_disk, vbr_partition, &hdr, read_status);
	}
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

bool is_non_fatal_floppy(uint8_t disk, uint8_t status)
{
	if (disk >= 0x80)
		return false;

	return status == ERR_NO_MBR || status == ERR_NO_LBA;
}

const char* err_desc(int n)
{
	static const char* arr[] =
	{
		"No error",
		"LBA is not supproted by disk",
		"No MBR found",
		"Partition not present",
		"Read error",
		"Uninitialized object",
		"Zero partitions found",
		"Invalid extended partition table",
		"GPT is of an incompatible type",
		"Corrupted partition table",
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
