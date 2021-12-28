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
//#include "io.h"



//void* memset(void* dst, int val, size_t count);
//void* memcpy(void* dest, const void* src, size_t count);

static bool is_extended_type(uint8_t type)
{
	uint8_t types[] = { 0x05, 0x0F, 0x85 };
	for (int i = 0; i < (int)sizeof(types); ++i)
		if (types[i] == type)
			return true;
	return false;
}


#define read_guard(...) { uint8_t status = read_drive_lba(__VA_ARGS__); if (status) { this->read_status = status; return ERR_READ; } }
#define err_guard(expr) { auto status = expr; if (status) return status; }

uint8_t disk_t::_init(uint8_t disk)
{
	memset(this, 0, sizeof(*this));
	this->bios_number = disk;
	uint8_t x, stos_mbr_size;

	{
		mbr_bootloader_t t;

		if (!drive_lba_supported(disk))
			return ERR_NO_LBA;

		if ((x = read_drive_lba(disk, 0, &t, 1)) != 0)
			return ERR_READ;

		if (t.signature != 0xAA55)
			return ERR_NO_MBR;

		this->has_mbr = true;

		if (strncmp(t.metadata.signature, "StOSboot", 8) == 0)
			this->stos_mbr = true;

		for (int i = 0; i < 4; ++i)
		{
			if (t.table[i].type == 0xEE)
			{
				if (this->has_gpt)
					return ERR_CORRUPTED_PARTITION_TABLE;

				gpt_header_t hdr;
				if ((x = read_drive_lba(disk, 1, &hdr, 1)) != 0)
				{
					this->read_status = x;
					continue;
				}
				if (strncmp(hdr.signature, "EFI PART", 8) == 0)
				{
					if (hdr.partition_table_entry_size != 128 ||
					    hdr.partition_table_begin != 2)
					    return ERR_NON_CONFORMING_GPT;
					this->has_gpt = true;
					this->partitions_number = hdr.partition_table_size;
					break;
				}
			}
		}

		stos_mbr_size = t.metadata.size;
	}

	do
	{
		if (!this->stos_mbr)
			break;

		if (this->has_gpt)
		{
			//TODO: search for global VBR in GPT
			const char stos_gpt_signature[] = "StOS bootloader ";
			static_assert(sizeof(stos_gpt_signature) == 17, "");

			{
				gpt_entry_t table[4];
				uint32_t current_lba = 1;
				for (uint32_t i = 0; i < this->partitions_number; ++i)
				{
					if ((i & 3) == 0)
						read_guard(disk, ++current_lba, &table, 1);
					if (strncmp(table[i & 3].type_guid, stos_gpt_signature, 16) == 0)
					{
						this->stos_vbr_partition = i;
						this->stos_vbr = true;
						break;
					}
				}
			}

			if (this->stos_vbr)
			{
				gpt_partition_info_t info;
				auto it = this->begin();

				it.partition_number = this->stos_vbr_partition;
				err_guard((uint8_t)it.get_gpt_info(&info));

				vbr_t t;
				read_guard(disk, info.first + stos_mbr_size - 1, &t, 1);
				if (strncmp(t.metadata.signature, "StOSload", 8) == 0)
					this->stos_vbr_version = t.metadata.version;
				else
				{
					this->stos_vbr = false;
					this->stos_vbr_partition = 0;
				}
			}
		}
		else
		{
			{
				vbr_t vbr;
				if ((x = read_drive_lba(disk, stos_mbr_size, &vbr, 1)) != 0)
				{
					this->read_status = x;
					break;
				}

				if (strncmp(vbr.metadata.signature, "StOSload", 8) != 0)
					continue;

				this->stos_vbr_partition = 0;
				this->stos_vbr = true;
				this->stos_vbr_version = vbr.metadata.version;
			}

			for (auto it = this->begin(); it.valid(); it.next())
			{
				mbr_partition_info_t info;
				auto x = (uint8_t)it.get_mbr_info(&info);
				if (x == ERR_NO_PARTITION)
					continue;

				err_guard(x);

				vbr_t vbr;
				read_guard(disk, info.lba, &vbr, 1);

				if (strncmp(vbr.metadata.signature, "StOSload", 8) != 0)
					continue;

				if (vbr.metadata.version <= this->stos_vbr_version)
					continue;

				this->stos_vbr_partition = it.partition_number;
				this->stos_vbr = true;
				this->stos_vbr_version = vbr.metadata.version;
			}
		}
	} while (false);


	return 0;
}
uint8_t disk_t::init(uint8_t disk)
{
	return this->init_status = this->_init(disk);
}
//int disk_t::store_mbr_partition_info(uint32_t n, partition_info_t* p) const;


#undef read_guard
#define read_guard(...) { uint8_t status = read_drive_lba(__VA_ARGS__); if (status) { this->disk->read_status = status; return ERR_READ; } }

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
		return ERR_UNINITIALIZED; //well in fact it is true only in corrupted objects, but close enough

	return 0;
}
uint32_t partition_iterator_t::get_mbr_info(mbr_partition_info_t* info) const
{
	mbr_entry_t result;
	auto x = this->get_mbr_entry(&result);
	if (x) return x;

	//TODO: account for extended partition should I?
	info->lba = result.start_lba;
	info->size = result.count_lba;
	info->type = result.type;
	info->active = result.active & 0x80;
	return x;
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
	return result->type ? 0 : ERR_NO_PARTITION;
}
uint32_t partition_iterator_t::get_gpt_info(gpt_partition_info_t* info) const
{
	gpt_entry_t e, *p = &e;
	auto x = this->get_gpt_entry(p);
	if (x) return x;

	memcpy(info->type, p->type_guid, 16);
	info->first = p->first_lba;
	info->last = p->last_lba;
	info->flags = p->flags;
	return x;
}
uint32_t partition_iterator_t::get_gpt_entry(gpt_entry_t* entry) const
{
	err_guard(this->_valid_for_partitions());

	if (!this->disk->has_gpt)
		return ERR_WRONG_PARTITION_TABLE_TYPE;

	gpt_entry_t table[4];
	read_guard(this->disk->bios_number, 2 + (this->partition_number >> 2), &table, 1);

	memcpy(entry, &table[this->partition_number & 3], sizeof(gpt_entry_t));
	return 0;
}
