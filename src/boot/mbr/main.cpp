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
#include "smol_vec.hpp"


typedef struct
{
	uint32_t partition = 0xFFFFFFFF;
	uint8_t disk = 0xFF;
	bool use_vbr : 1 = false;
} boot_entry_t;

typedef struct
{
	uint32_t partition;
	uint16_t version = 0;
	uint8_t disk;
} vbr_info_t;



smol_vec<boot_entry_t> boot_options;
bool with_errors = false;
disk_t* disks = nullptr;
int number_of_hdds;
int number_of_floppies;
#define disk_offset(x) ((x) <= number_of_floppies ? (x) : ((x) - 0x80 + number_of_floppies))



extern "C" uint32_t _invoke_vbr_helper(stos_request_header_t*, uint8_t disk, mbr_entry_t*);
uint32_t _boot(uint32_t n, const vbr_info_t&, uint8_t* read_status);
int next_disk(int);
int digits_count(uint32_t);
const char* err_desc(int);
uint32_t invoke_vbr(uint8_t disk, uint32_t partition, stos_request_header_t*, uint8_t* read_status);
bool is_non_fatal_floppy(uint8_t disk, uint8_t status);
void print_partition_info(uint8_t disk, uint32_t partition, uint32_t boot_number = 0, uint8_t tabulation = 0);
void add_boot_option(uint8_t disk, uint32_t partition, bool use_stos_vbr);
void process_error(const char* msg, uint32_t err_code, uint8_t read_err_code);
bool find_newest_vbr(vbr_info_t&);
const char* get_disk_size_string(uint8_t disk);



[[noreturn]]
void main()
{
	puts("StOS loader v1.0\n");

	number_of_hdds = get_disks_count();
	puts("Detected ");
	put32u(number_of_hdds);
	puts(" hard drive disk");
	if (number_of_hdds != 1)
		putc('s');
	endl();

	number_of_floppies = get_floppies_count();
	if (number_of_floppies)
	{
		puts("Detected ");
		put32u(number_of_floppies);
		puts(" floppy disk");
		if (number_of_floppies != 1)
			putc('s');
		endl();
	}

	endl();



	disks = (disk_t*)malloc((number_of_floppies + number_of_hdds) * sizeof(disk_t));

	for (int i = DISK_BEGIN; i != DISK_END; i = next_disk(i))
		disks[disk_offset(i)].init(i);

	vbr_info_t vbr;
	if (!find_newest_vbr(vbr))
	{
		puts("Warning: StOS global VBR not found in the system\n\n");
		with_errors = true;
	}



	boot_options.init(8);

	for (int i = DISK_BEGIN; i != DISK_END; i = next_disk(i))
	{
		disk_t& disk = disks[disk_offset(i)];

		if (disk.init_status == 0 && disk.has_mbr && disk.bios_number != get_boot_disk())
			add_boot_option(i, 0xFFFFFFFF, false);

		if (disk.init_status != 0)
		{
			if (!is_non_fatal_floppy(i, disk.init_status))
			{
				print_partition_info(i, -1);
				puts(": ");
				puts(err_desc(disk.init_status));
				endl();

				with_errors = true;
			}
			continue;
		}

		stos_boot_list_t* list = nullptr;
		uint32_t list_top = 0;

		if (vbr.version != 0)
		{
			stos_request_header_t header;
			header.struct_size = sizeof(header);
			header.request_number = STOS_REQ_GET_MBR_BOOT_OPTIONS;
			header.request_data = i;
			header.mbr_version = (uintptr_t)&MBR_VERSION;
			header.free_memory_begin = (uintptr_t)get_heap_top();

			uint8_t x8 = 0xFE;
			uint32_t x = invoke_vbr(vbr.disk, vbr.partition, &header, &x8);
			if (x != STOS_REQ_INVOKE_SUCCESS)
			{
				process_error("Subroutine invoke error: ", x, x8);
			}
			else
			{
				list = (stos_boot_list_t*)header.request_data;
			}
		}

		for (auto iter = disk.begin(); iter.valid(); iter.next())
		{
			stos_boot_list_t::entry_t* current_boot_list_entry = list ? &list->arr[list_top] : nullptr;
			bool in_list = (list && current_boot_list_entry->n == iter.partition_number);

			if (in_list && ++list_top == list->size)
			{
				list_top = 0;
				list = list->next;
			}

			uint32_t status;

			bool partition_present = false;
			bool active = false;
			if (disk.has_gpt)
			{
				static uint8_t zero16[16] = { 0 };

				gpt_entry_t info;
				status = iter.get_gpt_entry(&info);
				if (status == 0)
				{
					partition_present =
						memcmp(info.type_guid, zero16, 16) != 0 &&
						memcmp(info.partition_guid, zero16, 16) != 0;
					active = info.flags & GPT_FLAG_BIOS_BOOTABLE;
				}
			}
			else
			{
				mbr_entry_t info;
				status = iter.get_mbr_entry(&info);
				if (status == 0)
				{
					partition_present = info.type != 0;
					active = info.active & 0x80;
				}
			}

			if (!partition_present)
				continue;

			if (status == 0)
			{
				if (active || (in_list && current_boot_list_entry->status == bootability_status_t::bootable))
					add_boot_option(i, iter.partition_number, in_list);
			}
			else
			{
				print_partition_info(disk.bios_number, iter.partition_number + 1, 0);
				puts(": ");
				puts(err_desc(status));
				continue;
			}
		}
	}

	if (boot_options.size > 1 || (boot_options.size == 1 && with_errors))
	{
		int last_disk_number = -1;
		puts("Boot options list:\n");
		for (size_t i = 0; i < boot_options.size; ++i)
		{
			auto& current_option = boot_options[i];
			bool is_disk = current_option.partition == 0xFFFFFFFF;

			if ((int)current_option.disk != last_disk_number)
			{
				if (!is_disk)
				{
					print_partition_info(current_option.disk, -1);
					endl();
				}
				last_disk_number = current_option.disk;
			}
			print_partition_info(current_option.disk, current_option.partition, i + 1, (int)!is_disk);
			endl();
		}
	}
	else if (boot_options.size == 1)
	{
		puts("Single valid bootloader detected\n");
		uint8_t rd = 0xFE;
		uint32_t x;
		x = _boot(1, vbr, &rd);
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
		x = _boot(x, vbr, &x8);
		process_error("Boot error: ", x, x8);
	}
}



const char* get_disk_size_string(uint8_t disk)
{
	static const char* units[] =
	{ "B", "KiB", "MiB", "GiB" };

	static char str[28];

	uint64_t bytes = 0;

	if (disk_lba_supported(disk))
	{
		struct
		{
			uint16_t size;
			uint16_t flags;
			uint32_t num_phys_cylinders;
			uint32_t num_phys_heads;
			uint32_t num_phys_sectors;
			uint64_t sectors;
			uint16_t bytes_per_sector;
		} disk_data;

		disk_data.size = sizeof(disk_data);

		registers_info_t regs;
		regs.eax = 0x4800;
		regs.edx = disk;
		regs.esi = (uint32_t)&disk_data;

		interrupt(0x13, &regs);

		const uint8_t ah = uint8_t(regs.eax >> 8);
		if (ah == 0)
			bytes = disk_data.sectors * disk_data.bytes_per_sector;
	}

	if (bytes == 0)
	{
		registers_info_t regs;
		regs.eax = 0x15FF;
		regs.edx = disk | 0xFFFFFF00;
		regs.ecx = 0xFFFFFFFF;

		const uint16_t old_dx = (uint16_t)regs.edx;
		const uint16_t old_cx = (uint16_t)regs.ecx;

		interrupt(0x13, &regs);

		const uint16_t cx = uint16_t(regs.ecx);
		const uint16_t dx = uint16_t(regs.edx);
		const uint16_t ax = uint16_t(regs.eax);
		const uint8_t ah = uint8_t(ax >> 8);
		const bool is_valid_disk = (ah == 3) || (ax == 3);

		if (is_valid_disk && !(cx == old_cx && dx == old_dx))
			bytes = uint64_t(cx * 65536 + dx) * 512;
	}

	if (bytes == 0)
		return "unknown size";

	size_t unit_index = 0;
	const size_t max_unit = countof(units) - 1;
	uint64_t value = bytes;
	while (unit_index < max_unit && value >= 1024)
	{
		value = (value / 1024) + bool((value % 1024) >= 512);
		unit_index++;
	}

	char* p_buffer = str + countof(str);
	*--p_buffer = '\0';

	const char* unit = units[unit_index];
	size_t unit_name_length = strlen(unit);
	p_buffer -= unit_name_length;

	memcpy(p_buffer, unit, unit_name_length);
	*--p_buffer = ' ';

	while (value)
	{
		uint32_t q, r;
		divmod64_32(value, 10, &q, &r);
		*--p_buffer = char(r + '0');
		value = q;
	}

	return p_buffer;
}



bool find_newest_vbr(vbr_info_t& vbr)
{
	vbr.version = 0;

	for (int i = DISK_BEGIN; i != DISK_END; i = next_disk(i))
	{
		disk_t &disk = disks[disk_offset(i)];
		uint8_t x = disk.init_status;

		if (x != 0 && !is_non_fatal_floppy(i, x))
			with_errors = true;

		if (x == ERR_NO_MBR || x == ERR_NO_LBA)
			continue;

		if (!disk.stos_vbr)
			continue;

		if (disk.stos_vbr_version <= vbr.version)
			continue;

		vbr.disk = i;
		vbr.partition = disk.stos_vbr_partition;
		vbr.version = disk.stos_vbr_version;
	}

	return vbr.version != 0;
}



void add_boot_option(uint8_t disk, uint32_t partition, bool use_stos_vbr)
{
	boot_options.push_back({ partition, disk, use_stos_vbr });
};



void process_error(const char* msg, uint32_t err_code, uint8_t read_err_code)
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
	}
	endl();
	with_errors = true;
};



void print_partition_info(uint8_t disk, uint32_t partition, uint32_t boot_number, uint8_t tabulation)
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

	if (partition != 0xFFFFFFFF)
	{
		puts(" Partition ");
		put32u(partition);
	}
	else
	{
		puts(" (");
		puts(get_disk_size_string(disk));
		putc(')');
		if (disk == get_boot_disk())
			puts(" (current)");
	}
};



uint32_t invoke_vbr(uint8_t disk_n, uint32_t partition, stos_request_header_t* hdr, uint8_t* read_status)
{
	disk_t &disk = disks[disk_offset(disk_n)];
	if (disk.init_status != 0)
		return disk.init_status;

	uint64_t lba;
	uint32_t x; //last error
	uint8_t mbr_size = 0;


	{
		mbr_bootloader_t mbr;
		x = read_disk_lba(disk_n, 0, &mbr, 1);
		if (x) return ERR_READ;
		if (read_status) *read_status = disk.read_status;
		if (mbr.signature != 0xAA55 || strncmp(mbr.metadata.signature, "StOSboot", 8) != 0)
			return ERR_NO_MBR;
		mbr_size = mbr.metadata.size;
	}

	if (disk.has_gpt)
	{
		auto it = disk.begin();
		it.partition_number = partition;

		gpt_partition_info_t info;
		x = it.get_gpt_info(&info);
		if (read_status) *read_status = disk.read_status;
		if (x) return x;
		lba = info.first + mbr_size - 1;
	}
	else
	{
		if (partition != (uint32_t)-1)
		{
			auto it = disk.begin();
			it.partition_number = partition;

			mbr_partition_info_t info;
			x = it.get_mbr_info(&info);
			if (read_status) *read_status = disk.read_status;
			if (x) return x;
			lba = info.lba;
		}
		else
			lba = mbr_size;
	}

	x = read_disk_lba(disk_n, lba, (void*)0x7C00, 2);
	if (x && read_status) *read_status = x;
	if (x)
		return ERR_READ;

	mbr_entry_t entry;
	entry.active = disk_n;
	entry.start_lba = (uint32_t)lba;
	entry.count_lba = (uint32_t)(lba >> 32);

	return _invoke_vbr_helper(hdr, disk_n, &entry);
}


uint32_t _boot(uint32_t n, const vbr_info_t& vbr, uint8_t* read_status)
{
	auto& entry = boot_options[n - 1];

	if (entry.use_vbr)
	{
		stos_boot_req_data_t req_data;
		req_data.partition = entry.partition;
		req_data.disk = entry.disk;

		stos_request_header_t hdr;
		hdr.struct_size = sizeof(hdr);
		hdr.request_number = STOS_REQ_BOOT;
		hdr.request_data = (uint32_t)&req_data;
		hdr.free_memory_begin = (uint32_t)get_heap_top();
		return invoke_vbr(vbr.disk, vbr.partition, &hdr, read_status);
	}
	else
	{
		mbr_entry_t e;
		uint64_t lba = 0;
		uint8_t x;
		const bool have_actual_partition = entry.partition != 0xFFFFFFFF;

		if (have_actual_partition)
		{
			memset(&e, 0, sizeof(e));
			disk_t &disk = disks[disk_offset(entry.disk)];
			auto iter = disk.begin();
			iter.partition_number = entry.partition;

			if (disk.has_gpt)
			{
				gpt_entry_t info;
				x = iter.get_gpt_entry(&info);
				lba = info.first_lba;

				e.start_lba = (uint32_t)info.first_lba;
				e.count_lba = (uint32_t)(info.last_lba - info.first_lba + 1);
			}
			else
			{
				x = iter.get_mbr_entry(&e);
				lba = e.start_lba;
			}

			if (read_status)
				*read_status = disk.read_status;
			if (x)
				return x;
		}

		x = read_disk_lba(entry.disk, lba, (void*)0x7C00, 2);
		if (x && read_status) *read_status = x;
		if (x) return ERR_READ;

		e.active = entry.disk;
		_mbr_transfer_control_flow(&e);
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
		"No partitions found",
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
		if (x - 0x80 + 1 >= get_disks_count())
			return DISK_END;
	}

	return x + 1;
}
