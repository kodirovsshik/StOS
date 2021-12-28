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



#ifndef _DISK_HPP_
#define _DISK_HPP_


#include <stdint.h>

#include "mbr.h"
#include "gpt.h"
#include "defs.h"



#define DISK_END -1
#define DISK_BEGIN (get_floppies_count() != 0 ? 0 : (get_drives_count() != 0 ? 0x80 : DISK_END))
#define ERR_NO_LBA 1
#define ERR_NO_MBR 2
#define ERR_NO_PARTITION 3
#define ERR_READ 4
#define ERR_UNINITIALIZED 5
#define ERR_NO_PARTITIONS 6
#define ERR_INVALID_EBR 7
#define ERR_NON_CONFORMING_GPT 8
#define ERR_CORRUPTED_PARTITION_TABLE 9
#define ERR_WRONG_PARTITION_TABLE_TYPE 10



_EXTERN_C_

uint8_t get_drives_count();
uint8_t get_floppies_count();
uint8_t get_boot_drive();

uint8_t read_drive_lba(uint8_t disk, uint64_t lba, void* ptr, uint16_t sectors);
uint8_t write_drive_lba(uint8_t disk, uint64_t lba, const void* ptr, uint16_t sectors);

bool drive_lba_supported(uint8_t disk);

_EXTERN_C_END_




struct mbr_partition_info_t
{
	uint32_t lba = -1;
	uint32_t size = 0;
	uint8_t type;
	bool active : 1 = false;
};

struct gpt_partition_info_t
{
	char type[16];
	uint64_t first, last, flags;
};

struct disk_t;
struct partition_iterator_t
{
	disk_t* disk = nullptr;
	uint32_t partition_number = -1;

	void init(disk_t*);
	bool valid() const;
	void next();
	void reset();

	uint32_t get_mbr_info(mbr_partition_info_t*) const;
	uint32_t get_gpt_info(gpt_partition_info_t*) const;

	uint32_t get_mbr_entry(mbr_entry_t*) const;
	uint32_t get_gpt_entry(gpt_entry_t*) const;

private:
	void _next();
	int _valid_for_partitions() const;
};

struct disk_t
{
	uint32_t partitions_number = 0;
	uint32_t stos_vbr_partition = 0;
	uint16_t stos_vbr_version = 0;
	uint8_t bios_number = 0xFF;
	uint8_t init_status = 0xFF;
	uint8_t read_status = 0;
	bool has_mbr : 1 = false;
	bool has_gpt : 1 = false;
	bool stos_mbr : 1 = false;
	bool stos_vbr : 1 = false;

	uint8_t init(uint8_t disk);

	partition_iterator_t begin();

private:
	uint8_t _init(uint8_t);
};



#endif //!_DISK_HPP_
