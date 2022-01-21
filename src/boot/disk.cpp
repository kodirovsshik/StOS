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



//Huge o7 for my friend Саня who wasted his time money on trying to repair
//his ebook just to understand by the end that the thing does not have
//broken part being removable and it all went for nothing

#include <stdint.h>

#include "disk.hpp"
#include "gpt.h"




int disk_iterator_t::floppies_count = get_floppies_count();
int disk_iterator_t::disks_count = get_disks_count();

disk_iterator_t::disk_iterator_t()
{
	if (disk_iterator_t::floppies_count != 0)
		this->disk_number = 0;
	else if (disk_iterator_t::disks_count != 0)
		this->disk_number = 0x80;
	else
		this->disk_number = -1;
}

void disk_iterator_t::next()
{
	if (this->disk_number == -1)
		return;
	++this->disk_number;
	if (this->disk_number == disk_iterator_t::floppies_count)
		this->disk_number = 0x80;
	if (this->disk_number == 0x80 | disk_iterator_t::disks_count)
		this->disk_number = -1;
}
bool disk_iterator_t::valid() const
{
	return this->disk_number != -1;
}

uint8_t disk_iterator_t::operator*() const
{
	return (uint8_t)this->disk_number;
}




static bool is_extended_type(uint8_t type)
{
	uint8_t types[] = { 0x05, 0x0F, 0x85 };
	for (int i = 0; i < (int)sizeof(types); ++i)
		if (types[i] == type)
			return true;
	return false;
}


#define read_guard(...) { uint8_t status = read_disk_lba(__VA_ARGS__); if (status) { this->read_status = status; return ERR_READ; } }
#define err_guard(expr) { auto status = expr; if (status) return status; }

uint8_t disk_t::_init(uint8_t disk)
{
	memset(this, 0, sizeof(*this));
	this->bios_number = disk;
	//uint8_t x;

	{
		mbr_bootloader_t t;

		if (!disk_lba_supported(disk))
			return ERR_NO_LBA;

		read_guard(disk, 0, &t, 1);

		if (t.signature != 0xAA55)
			return ERR_NO_MBR;
		this->has_mbr = true;

		uint64_t extended_lba = 0;

		for (int i = 0; i < 4; ++i)
		{
			uint8_t type = t.table[i].type;
			if (type == 0)
				continue;
			if (!this->has_gpt)
				this->partitions_number = i + 1;
			if (type == 0xEE)
			{
				if (this->has_gpt)
					return ERR_CORRUPTED_PARTITION_TABLE;
				this->has_gpt = true;
			}
			else if (is_extended_type(type))
			{
				uint32_t lba = t.table[i].start_lba;
				if (lba == 0 || extended_lba != 0)
					return ERR_CORRUPTED_PARTITION_TABLE;
				extended_lba = lba;
			}
		}

		if (this->has_gpt)
		{
				gpt_header_t hdr;
				read_guard(disk, 1, &hdr, 1);

				if (memcmp(hdr.signature, "EFI PART", 8) == 0)
				{
					if (hdr.partition_table_entry_size != 128 || hdr.partition_table_begin != 2)
					    return ERR_NON_CONFORMING_GPT;
					this->partitions_number = hdr.partition_table_size;
					break;
				}
				else
					this->has_gpt = false;
		}

		uint32_t lba_offset = 0;
		if (!this->has_gpt && extended_lba != 0)
		{
			size_t n = 4;
			while (true)
			{
				read_guard(disk, extended_lba + lba_offset, &t, 1);
				if (t.table[0].type != 0)
					this->partitions_number = ++n;
				if (is_extended_type(t.table[1].type))
					lba_offset = t.table[1].start_lba;
				else
					break;
			}
		}
	}

	return 0;
}
uint8_t disk_t::init(uint8_t disk)
{
	return this->init_status = this->_init(disk);
}
//int disk_t::store_mbr_partition_info(uint32_t n, partition_info_t* p) const;


#undef read_guard
#define read_guard(...) { uint8_t status = read_disk_lba(__VA_ARGS__); if (status) { this->disk->read_status = status; return ERR_READ; } }

partition_iterator_t disk_t::begin()
{
	partition_iterator_t it;
	it.init(this);
	return it;
}

//o7 for Саня

void partition_iterator_t::init(disk_t* p)
{
	this->disk = p;
	this->reset();
}

void partition_iterator_t::reset()
{
	this->partition_number = -1;
	this->_next();
}
bool partition_iterator_t::valid() const
{
	if (this->disk == nullptr)
		return false;
	if (this->partition_number == (uint32_t)-1)
		return false;
	return true;
}
void partition_iterator_t::next()
{
	if (!this->valid())
		return;
	this->_next();
}

void partition_iterator_t::_next()
{
	if (++this->partition_number >= this->disk->partitions_number)
		this->partition_number = -1;
}

int partition_iterator_t::_valid_for_partitions() const
{
	if (!this->valid())
		return ERR_UNINITIALIZED;

	if (!this->disk->has_mbr)
		return ERR_NO_MBR;

	if (this->disk->partitions_number == 0)
		return ERR_NO_PARTITIONS;

	if (this->partition_number >= this->disk->partitions_number)
		return ERR_NO_PARTITION;

	return 0;
}
}
uint32_t partition_iterator_t::get_mbr_entry(mbr_entry_t* info) const
{
	err_guard(this->_valid_for_partitions());

	if (this->disk->has_gpt)
		return ERR_WRONG_PARTITION_TABLE_TYPE;

	mbr_t mbr;
	read_guard(this->disk->bios_number, 0, &mbr, 1);

	mbr_entry_t* result = nullptr;

	if (this->partition_number < 4)
		result = &mbr.table[this->partition_number];
	else
	{
		uint64_t extended_lba = 0;
		for (int i = 0; i < 4; ++i)
		{
			if (is_extended_type(mbr.table[i].type))
			{
				extended_lba = mbr.table[i].start_lba;
				break;
			}
		}
		if (extended_lba == 0)
			return ERR_NO_PARTITION;

		read_guard(this->disk->bios_number, extended_lba, &mbr, 1);

		for (uint32_t i = 3; i < this->partition_number; ++i)
		{
			if (!is_extended_type(mbr.table[1].type))
				return ERR_INVALID_EBR;
			read_guard(this->disk->bios_number, extended_lba + mbr.table[i].start_lba, &mbr, 1);
		}

		result = &mbr.table[0];
	}

	memcpy(info, result, sizeof(mbr_entry_t));
	return 0;
}
uint32_t partition_iterator_t::get_gpt_entry(gpt_entry_t* entry) const
{
	err_guard(this->_valid_for_partitions());

	if (!this->disk->has_gpt)
		return ERR_WRONG_PARTITION_TABLE_TYPE;

	gpt_entry_t table[4];
	read_guard(this->disk->bios_number, 2 + (this->partition_number / 4), &table, 1);

	memcpy(entry, &table[this->partition_number % 4], sizeof(gpt_entry_t));
	return 0;
}
