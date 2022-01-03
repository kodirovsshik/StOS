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
#define _ERR_FILE_DRIVE 3
#define _ERR_FILE_MBR 4
#define _ERR_FILE_VBR 5
#define _ERR_NO_FIT_PART 6
#define _ERR_NO_GPT 7
#define _ERR_READ 8
#define _ERR_INTERNAL 9
#define _ERR_NO_STOS_MBR 10
#define _ERR_ARGS 11
#define _ERR_WRITE 12



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
	FILE* drives[256] = {0};

	FILE* disk_find(uint8_t n)
	{
		return drives[n];
	}

	uint8_t read_drive_lba(uint8_t disk, uint64_t lba, void* ptr, uint16_t sectors)
	{
		uint64_t size = (uint32_t)sectors * 512;
		uint64_t pos = lba * 512;
		if (fseek(disk_find(disk), pos, SEEK_SET) != 0)
			return 1;
		return fread(ptr, 1, size, disk_find(disk)) != size;
	}
	uint8_t write_drive_lba(uint8_t disk, uint64_t lba, const void* ptr, uint16_t sectors)
	{
		uint64_t size = (uint32_t)sectors * 512;
		uint64_t pos = lba * 512;
		if (fseek(disk_find(disk), pos, SEEK_SET) != 0)
			return 1;
		return fwrite(ptr, 1, size, disk_find(disk)) != size;
	}

	bool drive_lba_supported(uint8_t)
	{
		return true;
	}

	uint8_t disk_get(FILE* f)
	{
		for (int i = 0; i < 256; ++i)
		{
			if (drives[i] == f)
				return (uint8_t)i;
		}

		for (int i = 0; i < 256; ++i)
		{
			if (drives[i] == 0)
			{
				drives[i] = f;
				return (uint8_t)i;
			}
		}

		fprintf(stderr, "Too many disks allocated");
		exit(_ERR_INTERNAL);
	}

	void disk_free(uint8_t index, FILE* f)
	{
		check(drives[index] == f, _ERR_INTERNAL, "Bad drive deallocation");
		drives[index] = 0;
	}
}


const char* argv0 = nullptr;

const char* usage_mbr =
R"(MBR command - use to write a bootloader to a disk:
	%s mbr my/drive mbr.img
)";

const char* usage_vbr1 =
R"(VBR command - use to write a vbr to a disk:
	%s vbr my/drive vbr.img
)";

const char* usage_vbr2 =
R"(VBR command - use to write a vbr to partition N:
	%s vbr my/drive vbr.img N
)";

const char* usage_cmd =
R"(cmd command - use to execute a command inside partition N's root directory:
	%s cmd command
)";

void usage()
{
	printf("Usage: %s comand path/to/disk command_args\n", argv0);
	printf(usage_mbr, argv0);
	printf(usage_vbr1, argv0);
	printf(usage_vbr2, argv0);
}

int command_mbr(int argc, const char* const* argv)
{
	check(argc == 2, _ERR_ARGS_COUNT, usage_mbr, argv0);

	FILE* f_disk = fopen(argv[0], "rb+");
	FILE* f_mbr = fopen(argv[1], "rb");

	check(f_disk, _ERR_FILE_DRIVE, "Failed to open %s", argv[0]);
	check(f_mbr,  _ERR_FILE_MBR,   "Failed to open %s", argv[1]);

	mbr_bootloader_t mbr, mbr_disk;
	vbr_t& vbr_disk = *(vbr_t*)&mbr_disk, &vbr = *(vbr_t*)&mbr;
	check(fread(&mbr,      1, 512, f_mbr)  == 512, _ERR_READ, "Failed to read from %s", argv[1]);
	check(fread(&mbr_disk, 1, 512, f_disk) == 512, _ERR_READ, "Failed to read from %s", argv[1]);

	check(mbr_disk.signature == 0xAA55, _ERR_NO_MBR, "%s does not have an MBR", argv[0]);
	check(     mbr.signature == 0xAA55, _ERR_NO_MBR, "%s is not an MBR image", argv[1]);

	fseek(f_disk, 0, SEEK_SET);
	disk_t disk;
	if (disk.init(disk_get(f_disk)) != 0)
		check(false, disk.init_status | 0x40000000, "%s initialization failed", argv[0]);

	fseek(f_disk, 0, SEEK_SET);
	memcpy(vbr_disk.code1, vbr.code1, sizeof(vbr_t::code1));
	memcpy(vbr_disk.code2, vbr.code2, sizeof(vbr_t::code2));
	memcpy(&vbr_disk.metadata, &vbr.metadata, sizeof(vbr_t::metadata));
	fwrite(&vbr_disk, 1, 512, f_disk);

	if (disk.has_gpt)
	{
		static const char stos_gpt_signature[] = "StOS bootloader ";
		uint32_t partition = 0xFFFFFFFF;

		for (auto iter = disk.begin(); iter.valid(); iter.next())
		{
			gpt_entry_t dst;
			if (iter.get_gpt_entry(&dst) != 0)
			{
				printf("WARNING: failed to probe %s for partition %u\n", argv[0], iter.partition_number);
				continue;
			}

			if (strncmp(dst.type_guid, stos_gpt_signature, 16) == 0)
			{
				partition = iter.partition_number;
				printf("INFO: selecting partition %u\n", partition);
			}
		}

		if (partition == 0xFFFFFFFF)
		{
			fprintf(stderr, "No situable partition found on %s\n", argv[0]);
			return _ERR_NO_FIT_PART;
		}
		else
		{
			gpt_entry_t dst;
			auto iter = disk.begin();
			iter.partition_number = partition;
			iter.get_gpt_entry(&dst) == 0;
			fseek(f_disk, long(512) * dst.first_lba, SEEK_SET);
		}
	}

	while (feof(f_mbr) == 0)
	{
		fread(&mbr, 1, 512, f_mbr);
		fwrite(&mbr, 1, 512, f_disk);
	}

	return 0;
}

int command_vbr(int argc, const char* const* argv)
{
	if (argc < 2 || argc > 3)
	{
		printf(usage_vbr1, argv0);
		printf(usage_vbr2, argv0);
		return _ERR_ARGS_COUNT;
	}

	uint32_t partition = 0xFFFFFFFF;
	if (argc == 3)
	{
		check(sscanf(argv[2], "%d", &partition) == 1 && partition != 0xFFFFFFFF,
			_ERR_ARGS, "Invalid partition number: %s", partition);
	}

	FILE* f_disk = fopen(argv[0], "rb+");
	FILE* f_vbr = fopen(argv[1], "rb");

	check(f_disk, _ERR_FILE_DRIVE, "Failed to open %s", argv[0]);
	check(f_vbr, _ERR_FILE_VBR, "Failed to open %s", argv[1]);

	//disk_number_X
	uint8_t dn_disk = disk_get(f_disk);

	//disk_X
	disk_t d_disk;
	d_disk.init(dn_disk);

	check(d_disk.init_status == 0, d_disk.init_status | 0x40000000, "%s initialization failed", argv[0]);

	check(d_disk.stos_mbr, _ERR_NO_STOS_MBR, "StOS VBR can only be installed on disks with StOS MBR");


	if (d_disk.has_gpt)
	{
		static const char stos_gpt_signature[] = "StOS bootloader ";

		if (partition == 0xFFFFFFFF)
		{
			for (auto iter = d_disk.begin(); iter.valid(); iter.next())
			{
				gpt_entry_t dst;
				if (iter.get_gpt_entry(&dst) != 0)
				{
					printf("WARNING: failed to probe %s for partition %u\n", argv[0], iter.partition_number);
					continue;
				}

				if (strncmp(dst.type_guid, stos_gpt_signature, 16) == 0)
				{
					partition = iter.partition_number;
					printf("INFO: selecting partition %u\n", partition);
				}
			}
		}

		check(partition == 0xFFFFFFFF, _ERR_NO_FIT_PART, "Failed to find a partition on %s suitable for StOS VBR", argv[0]);

		gpt_entry_t part;
		{
			auto iter = d_disk.begin();
			iter.partition_number = partition;
			check(iter.get_gpt_entry(&part) == 0, _ERR_ARGS, "Failed to probe %s for partition %u", argv[0], partition);

			check(strncmp(part.type_guid, stos_gpt_signature, 16) == 0,
				_ERR_NO_FIT_PART, "Selected partition is not suitable for StOS VBR");
		}

		mbr_bootloader_t mbr;
		fseek(f_disk, 0, SEEK_SET);
		check(fread(&mbr, 1, 512, f_disk) == 512, _ERR_READ, "Failed to read %s", argv[0]);
		check(fseek(f_disk, (mbr.metadata.size - 1 + (long)part.first_lba) * 512, SEEK_SET) == 0,
			_ERR_FILE_DRIVE, "Seek error on %s", argv[0]);

		fseek(f_vbr, 0, SEEK_SET);
		while (feof(f_vbr) == 0)
		{
			check(fread(&mbr, 1, 512, f_vbr) != 0, _ERR_READ, "Failed to read %s", argv[1]);
			check(fwrite(&mbr, 1, 512, f_disk) == 512, _ERR_WRITE, "Failed to write %s", argv[0]);
		}
	}
	else
	{
		uint32_t lba;
		if (partition != 0xFFFFFFFF)
		{
			check(partition <= d_disk.partitions_number, _ERR_NO_FIT_PART, "%s has no partition %u", argv[0], partition);

			auto iter = d_disk.begin();
			iter.partition_number = partition - 1;
			check(iter.valid(), _ERR_NO_FIT_PART, "Failed to find partition %s on %u", partition, argv[0]);

			mbr_entry_t part;
			check(iter.get_mbr_entry(&part) == 0, _ERR_NO_FIT_PART, "Failed to find partition %s on %u", partition, argv[0]);
			lba = part.start_lba;
		}
		else
		{
			fseek(f_disk, 0, SEEK_SET);
			mbr_bootloader_t mbr;
			check(fread(&mbr, 1, 512, f_disk) == 512, _ERR_READ, "Failed to read %s", argv[1]);
			lba = mbr.metadata.size;
		}

		uint8_t buffer[512];

		//printf("LBA: %u\n", lba);
		fseek(f_disk, lba * (long)512, SEEK_SET);
		fseek(f_vbr, 0, SEEK_SET);
		while (feof(f_vbr) == 0)
		{
			check(fread(buffer, 1, 512, f_vbr) != 0, _ERR_READ, "Failed to read %s", argv[1]);
			check(fwrite(buffer, 1, 512, f_disk) == 512, _ERR_WRITE, "Failed to write %s", argv[0]);
		}
	}

	disk_free(dn_disk, f_disk);
	return 0;
}

int command_cmd(int argc, const char* const* argv)
{
	return -1;
}

int main(int argc, const char* const* argv)
{
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
