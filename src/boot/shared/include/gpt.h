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



#ifndef _GPT_H_
#define _GPT_H_



#include <stdint.h>



typedef struct
{
	char signature[8];
	uint32_t revision;
	uint32_t header_size;
	uint32_t header_crc;
	uint32_t reserved;
	uint64_t header_current_lba;
	uint64_t header_reserved_lba;
	uint64_t usable_lba_begin;
	uint64_t usable_lba_end;
	char guid[16];
	uint64_t partition_table_begin;
	uint32_t partition_table_size;
	uint32_t partition_table_entry_size;
	uint32_t partition_table_crc;
	char padding[0x1A4];
} gpt_header_t;

static_assert(sizeof(gpt_header_t) == 512);



typedef struct
{
	char type_guid[16];
	char partition_guid[16];
	uint64_t first_lba;
	uint64_t last_lba;
	uint64_t flags;
	uint16_t name[36];
} gpt_entry_t;

static_assert(sizeof(gpt_entry_t) == 128);



#endif //!_GPT_H_
