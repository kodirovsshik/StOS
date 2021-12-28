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



#ifndef _MBR_H_
#define _MBR_H_



#include <stdint.h>

#include "defs.h"



typedef struct
{
	uint8_t head;
	uint8_t sec : 6;
	uint16_t cyl : 10;
} PACKED chs_t;

static_assert(sizeof(chs_t) == 3);



typedef struct
{
	uint8_t active;
	chs_t start_chs;
	uint8_t type;
	chs_t end_chs;
	uint32_t start_lba;
	uint32_t count_lba;
} PACKED mbr_entry_t;

static_assert(sizeof(mbr_entry_t) == 16);



typedef struct
{
	uint8_t code[440];
	uint8_t uid[6];
	mbr_entry_t table[4];
	union
	{
		uint8_t sig[2];
		uint16_t signature;
	};
} PACKED mbr_t;

static_assert(sizeof(mbr_t) == 512);



typedef struct
{
	uint8_t code[0x1AD];

	struct
	{
		char signature[8];

		union
		{
			uint8_t verl, verh;
			uint16_t version;
		};

		uint8_t size;

	} PACKED metadata;

	uint8_t uid[6];
	mbr_entry_t table[4];
	union
	{
		uint8_t sig[2];
		uint16_t signature;
	};
} PACKED mbr_bootloader_t;

static_assert(sizeof(mbr_bootloader_t) == 512);



typedef struct
{
	uint8_t code1[3];
	uint8_t oem[8];
	uint8_t bpb[79];
	uint8_t code2[0x153];

	struct
	{
		char signature[8];

		union
		{
			uint8_t verl, verh;
			uint16_t version;
		};

		uint8_t size;

	} PACKED metadata;

	uint8_t code3[70];

	union
	{
		uint8_t sig[2];
		uint16_t signature;
	};
} PACKED vbr_t;

static_assert(sizeof(vbr_t) == 512);



#endif //!_MBR_H_
