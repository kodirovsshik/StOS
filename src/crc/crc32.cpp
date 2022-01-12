#if 0
	  This file is a part of StOS project - a small operating system
	  	made for learning purposes
	  Copyright (C) 2021 Egorov Stanislav, kodirovsshik@mail.ru, kodirovsshik@gmail.com
	  CRC lookuo table is a subject to public domain

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



#include <stdint.h>
#include <stddef.h>



extern "C"
uint32_t crc32_init()
{
	return 0;
}

extern "C"
uint32_t crc32_update(const void* data, size_t size, uint32_t crc)
{
	crc = ~crc;

	auto* p = (const uint8_t*)data, *pe = p + size;

	static constexpr uint32_t crc_polynomial = 0xEDB88320;
	static constexpr uint32_t lookup[2] = { 0x00000000, crc_polynomial };
	//1 bit lookup table here we go

	while (p < pe)
	{
		crc ^= *p++;
		for (int i = 8; i > 0; --i)
			crc = (crc >> 1) ^ lookup[crc & 1];
	}

	return ~crc;
}

extern "C"
uint32_t crc32(const void* data, size_t size)
{
	return crc32_update(data, size, crc32_init());
}
