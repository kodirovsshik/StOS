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
#include "io.h"


void* memset(void* dst, int val, size_t count);
void* memcpy(void* dest, const void* src, size_t count);


uint8_t disk_t::_init(uint8_t disk)
{
	memset(this, 0, sizeof(*this));
	this->bios_number = disk;
	uint8_t x;

	{
		mbr_bootloader_t t;

		if (!drive_lba_supported(disk))
			return ERR_NO_LBA;

		if ((x = read_drive_lba(disk, 0, &t, 1)) != 0)
			return ERR_READ;

		if (t.signature != 0xAA55)
			return ERR_NO_MBR;

		this->has_mbr = true;

		for (int i = 0; i < 3; ++i)
		{
			if (t.table[i] == 0xEE)
			{
				gpt_header_t hdr;
				if ((x = read_drive_lba(disk, 1, &hdr, 1)) != 0)
				{
					this->read_status = x;
					continue;
				}
			}
			if (strncmp(hdr.signature, "EFI PART", 8) == 0)
			{
				this->has_gpt = true;
				break;
			}
		}

		do
		{
			if (this->has_gpt)
			{
				//TODO: search for global VBR in GPT
			}
			else
			{
				if (strncmp(t.metadata.signature, "StOSboot", 8) != 0)
					break;

				this->stos_mbr = true;
				vbr_t vbr;
				if ((x = read_drive_lba(disk, t.metadata.size, &vbr, 1)) != 0)
				{
					this->read_status = x;
					break;
				}

				this->stos_vhbr_partition = 0;
				this->stos_vbr = true;
				this->stos_vbr_version = t.metadata.version;
			}
		} while (false);
	}
}
uint8_t disk_t::init(uint8_t disk)
{
	return this->init_status = this->_init(disk);
}
int disk_t::store_mbr_partition_info(uint32_t n, partition_info_t* p) const;

partition_iterator_t disk_t::begin() const
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
	this->partirion_number = 0;
	this->_next();
}
bool partition_iterator_t::valid() const
{
	if (this->disk == nullptr)
		return false;
	if (this->partirion_number == (uint32_t)-1)
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
		this->partitions_number = -1;
}
