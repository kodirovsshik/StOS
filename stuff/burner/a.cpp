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



#include "disk.hpp"
#include "mbr.h"
#include "gpt.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <utility>


#define _ERR_ARGS_COUNT 1
#define _ERR_NO_MBR 2
#define _ERR_FILE_OPEN 3
#define _ERR_WRITE 4
#define _ERR_READ 5
#define _ERR_NO_FIT_PART 6
#define _ERR_NO_GPT 7
#define _ERR_ARGS 8
#define _ERR_INTERNAL 9
#define _ERR_NO_STOS_MBR 10



void check(bool true_expr, int code, const char* fmt = "", ...)
{
	if (!true_expr)
	{
		va_list ap;
		va_start(ap, fmt);

		vfprintf(stderr, fmt, ap);
		fputc('\n', stderr);

		va_end(ap);
		exit(code);
	}
}

extern "C"
{
	FILE* disks[256] = {0};

	FILE* disk_find(uint8_t n)
	{
		return disks[n];
	}

	uint8_t read_disk_lba(uint8_t disk, uint64_t lba, void* ptr, uint16_t sectors)
	{
		uint64_t size = (uint32_t)sectors * 512;
		uint64_t pos = lba * 512;
		if (fseek(disk_find(disk), pos, SEEK_SET) != 0)
			return 1;
		return fread(ptr, 1, size, disk_find(disk)) != size;
	}
	uint8_t write_disk_lba(uint8_t disk, uint64_t lba, const void* ptr, uint16_t sectors)
	{
		uint64_t size = (uint32_t)sectors * 512;
		uint64_t pos = lba * 512;
		if (fseek(disk_find(disk), pos, SEEK_SET) != 0)
			return 1;
		return fwrite(ptr, 1, size, disk_find(disk)) != size;
	}

	bool disk_lba_supported(uint8_t)
	{
		return true;
	}

	uint8_t disk_get(FILE* f)
	{
		for (int i = 0; i < 256; ++i)
		{
			if (disks[i] == f)
				return (uint8_t)i;
		}

		for (int i = 0; i < 256; ++i)
		{
			if (disks[i] == 0)
			{
				disks[i] = f;
				return (uint8_t)i;
			}
		}

		fprintf(stderr, "Too many disks allocated");
		exit(_ERR_INTERNAL);
	}

	void disk_free(uint8_t index, FILE* f)
	{
		check(disks[index] == f, _ERR_INTERNAL, "Bad disk deallocation");
		disks[index] = 0;
	}
}

#define EVAL(x) x
#define _io_guard(func, line, err_code, err_message, ...) io_guard(func, err_code, __FILE__ ":" #line ": " err_message, __VA_ARGS__)
#define read_guard(...) _io_guard(fread, EVAL(__LINE__), _ERR_READ, "Read error", __VA_ARGS__)
#define write_guard(...) _io_guard(fwrite, EVAL(__LINE__), _ERR_WRITE, "Write error", __VA_ARGS__)

template<class callable_t, class T>
void io_guard(callable_t&& io_func, int err_code, const char* err_msg, uint8_t disk, uint64_t lba, T* ptr, uint16_t sectors)
{
	FILE* fd = disk_find(disk);
	check(fd != nullptr, _ERR_INTERNAL, "%s\n%s\n", "Disk-file translation error", err_msg);
	fseek(fd, (long)512 * lba, SEEK_SET);

	uint64_t bytes = sectors * (uint64_t)512;
	size_t result = io_func(ptr, 1, bytes, fd);
	check(ferror(fd) == 0, err_code, err_msg);
	if ((void*)io_func == (void*)fwrite)
		fflush(fd);
	else
		memset((uint8_t*)ptr + result, 0, bytes - result);
}

const char* argv0 = nullptr;

const char* usage_mbr =
R"(MBR command - use to write a bios bootloader to a disk
	(or partition N for gpt disks only):
	%s mbr my/disk mbr.img [N]
)";

const char* usage_vbr =
R"(VBR command - use to write a vbr to partition N:
	%s vbr my/disk vbr.img [N]
)";

const char* usage_cmd =
R"(cmd command - use to execute a command inside partition N's root directory:
	%s cmd command
)";

void usage()
{
	printf("Usage: %s comand path/to/disk command_args\n", argv0);
	printf(usage_mbr, argv0);
	printf(usage_vbr, argv0);
}

bool is_mbr_present(uint8_t disk)
{
	mbr_t mbr;
	read_guard(disk, 0, &mbr, 1);

	return mbr.signature == 0xAA55;
}

void ensure_mbr_present(uint8_t disk, const char* name)
{
	check(is_mbr_present(disk), _ERR_NO_MBR, "No MBR found on %s", name);
}

void copy_sector_selective(bool is_vbr, uint8_t dst, uint8_t src, uint64_t dst_lba, uint64_t src_lba)
{
	vbr_t dst_sector, src_sector;

	read_guard(dst, dst_lba, &dst_sector, 1);
	read_guard(src, src_lba, &src_sector, 1);

	memcpy(&dst_sector.code1, &src_sector.code1, sizeof(vbr_t::code1));
	memcpy(&dst_sector.code2, &src_sector.code2, sizeof(vbr_t::code2));
	memcpy(&dst_sector.metadata, &src_sector.metadata, sizeof(vbr_t::metadata));

	//for MBR sectors, this place differs from VBR sectors and is occupied
	//a by partition table and disk id and we would not want to overwrite it
	if (is_vbr)
		memcpy(&dst_sector.code3, &src_sector.code3, sizeof(vbr_t::code2));

	memcpy(&dst_sector.signature, &src_sector.signature, sizeof(vbr_t::signature));

	write_guard(dst, dst_lba, &dst_sector, 1);
}
void disk_copy(uint8_t dst, uint8_t src, uint64_t dst_lba, uint64_t src_lba, size_t count)
{
	uint8_t buffer[512];

	FILE* f_src;
	if (count == (size_t)-1)
	{
		f_src = disk_find(src);
		fseek(f_src, 0, SEEK_SET);
	}

	while (true)
	{
		if (count == (size_t)-1)
		{
			if (feof(f_src))
				break;
		}
		else if (count-- == 0)
			break;

		 read_guard(src, src_lba++, &buffer, 1);
		write_guard(dst, dst_lba++, &buffer, 1);
	}
}

uint32_t get_mbr_size(uint8_t disk)
{
	mbr_bootloader_t mbr;
	read_guard(disk, 0, &mbr, 1);
	check(strncmp(mbr.metadata.signature, "StOSboot", 8) == 0,
		_ERR_NO_STOS_MBR,
		"StOS vbr can only be installed on a disk with a StOS bootloader");

	return mbr.metadata.size;
}
uint64_t get_disk_sectors_count(uint8_t disk)
{
	FILE* f = disk_find(disk);
	fseek(f, 0, SEEK_END);
	uint64_t size = (uint64_t)ftell(f);
	return size / 512 + bool(size % 512);
}
int command_mbr_vbr_for_mbr_disk(uint32_t selected_partition, disk_t& disk, uint8_t image_disk_number, bool is_vbr_image)
{
	uint64_t dst_lba;
	if (selected_partition != 0xFFFFFFFF)
	{
		check(is_vbr_image, _ERR_ARGS, "StOS bootloader can not be installed to a partition");

		auto iter = disk.begin();
		iter.partition_number = selected_partition;

		mbr_entry_t partition_info;
		check(iter.get_mbr_entry(&partition_info) == 0, _ERR_INTERNAL,
			"Failed to probe for partition %u", selected_partition);

		dst_lba = partition_info.start_lba;
	}
	else
	{
		if (is_vbr_image)
			dst_lba = get_mbr_size(disk.bios_number);
		else
			dst_lba = 0;
	}

	copy_sector_selective(is_vbr_image, disk.bios_number, image_disk_number, dst_lba, 0);
	disk_copy(disk.bios_number, image_disk_number, dst_lba + 1, 1, -1);
	return 0;
}

uint32_t find_stos_boot_partition(disk_t& disk, gpt_entry_t& partition_info)
{
	for (auto iter = disk.begin(); iter.valid(); iter.next())
	{
		if (iter.get_gpt_entry(&partition_info) != 0)
		{
			fprintf(stderr, "Warning: failed to probe disk for partition %u\n", iter.partition_number);
			continue;
		}

		if (memcmp(partition_info.type_guid, "StOS bootloader ", 16) == 0)
		{
			fprintf(stderr, "Warning: selecting partition %u\n", iter.partition_number);
			return iter.partition_number;
		}
	}

	check(true == false, _ERR_NO_FIT_PART, "Failed to find partition of type \"StOS bootloader \"");
	return 0;
}

bool is_stos_boot_partition(const gpt_entry_t& partition_info)
{
	return memcmp(partition_info.type_guid, "StOS bootloader ", 16) == 0;
}

int command_mbr_vbr_for_gpt_disk(uint32_t selected_partition, disk_t& disk, uint8_t image_disk_number, bool is_vbr_image)
{
	gpt_entry_t partition_info;
	if (selected_partition == (uint32_t)-1)
		selected_partition = find_stos_boot_partition(disk, partition_info);
	else
	{
		auto iter = disk.begin();
		iter.partition_number = selected_partition;

		check(iter.get_gpt_entry(&partition_info) == 0, _ERR_NO_FIT_PART,
			"failed to probe disk for partition %u\n", iter.partition_number);

		if (!is_vbr_image)
			check(is_stos_boot_partition(partition_info),	_ERR_NO_FIT_PART,
				"Selected partition must have type GUID \"StOS bootloader \"");
	}

	uint64_t dst_lba = partition_info.first_lba, src_lba;
	if (is_vbr_image)
	{
		src_lba = 0;
		if (is_stos_boot_partition(partition_info))
			dst_lba += get_mbr_size(disk.bios_number) - 1;
	}
	else
	{
		copy_sector_selective(is_vbr_image, disk.bios_number, image_disk_number, 0, 0);
		src_lba = 1;
	}

	uint64_t partition_size_sectors = partition_info.last_lba + 1 - dst_lba - src_lba;
	check(partition_size_sectors >= get_disk_sectors_count(image_disk_number),
		_ERR_NO_FIT_PART, "Selected partition is too small");

	disk_copy(disk.bios_number, image_disk_number, dst_lba, src_lba, -1);
	return 0;
}

int command_mbr_vbr(const bool is_vbr_image, int argc, const char* const* argv)
{
	if (argc < 2 || argc > 3)
	{
		printf(usage_mbr, argv0);
		printf(usage_vbr, argv0);
		return _ERR_ARGS_COUNT;
	}

	uint32_t partition_number = -1;

	FILE* f_disk = fopen(argv[0], "rb+");
	FILE* f_image = fopen(argv[1], "rb");
	if (argc == 3)
		check(sscanf(argv[2], "%u", &partition_number) == 1 &&
			partition_number != (uint32_t)-1, _ERR_ARGS,
			"Invalid partition number: %s", argv[2]
		);

	uint8_t
		disk_number_for_disk_file = disk_get(f_disk),
		disk_number_for_image = disk_get(f_image);

	ensure_mbr_present(disk_number_for_disk_file, argv[0]);
	ensure_mbr_present(disk_number_for_image, argv[1]);

	disk_t disk_for_disk_file;
	disk_for_disk_file.init(disk_number_for_disk_file);

	check(disk_for_disk_file.init_status == 0,
		0x40000000 | disk_for_disk_file.init_status,
		"Disk initialization failure on %s", argv[0]);

	if (disk_for_disk_file.has_gpt)
		return command_mbr_vbr_for_gpt_disk(partition_number, disk_for_disk_file, disk_number_for_image, is_vbr_image);
	else
		return command_mbr_vbr_for_mbr_disk(partition_number, disk_for_disk_file, disk_number_for_image, is_vbr_image);
}

int command_vbr(int argc, const char* const* argv)
{
	return command_mbr_vbr(true, argc, argv);
}
int command_mbr(int argc, const char* const* argv)
{
	return command_mbr_vbr(false, argc, argv);
}
#define USE (void)
int command_cmd(int argc, const char* const* argv)
{
	USE argc;
	USE argv;
	return -1;
}

int main(int argc, const char* const* argv)
{
	fclose(stderr);
	argv0 = argv[0];

	if (argc < 2)
	{
		usage();
		return _ERR_ARGS_COUNT;
	}

	const std::pair<const char*, int(*)(int, const char* const*)> handlers[] =
	{
		{ "mbr", command_mbr },
		{ "vbr", command_vbr },
		{ "cmd", command_cmd },
	};

	for (const auto& [cmd, handler] : handlers)
	{
		if (strcmp(cmd, argv[1]) == 0)
			return handler(argc - 2, argv + 2);
	}

	usage();
	return _ERR_ARGS_COUNT;
}
