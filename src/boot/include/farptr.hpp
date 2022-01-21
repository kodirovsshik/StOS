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



#include "defs.h"


void segoff_panic_func(const void* arg, const void* instance);


template<class T>
class farptr_t
{
public:
	uint16_t offset;
	uint16_t segment;

	farptr_t() {}
	farptr_t(T* p)
	{
		uint32_t x = (uint32_t)p;
		if (x > 0x10FFEF)
			segoff_panic_func(p, this);

		this->segment = (x >> 4) & 0xFFFF;
		this->offset = x & 0xF;
	}

	operator T*() const
	{
		return (T*)((this->segment << 4) + this->offset);
	}

	/*
	T& operator[](size_t n) const
	{
		return ((T*)*this)[n];
	}

	T& operator*() const
	{
		return *(T*)*this;
	}

	T* operator+(ptrdiff_t d) const
	{
		return (T*)*this + d;
	}
	T* operator-(ptrdiff_t d) const
	{
		return (T*)*this + d;
	}
	*/

	operator uint32_t() const
	{
		return uint32_t(this->operator T*());
	}

	uint32_t pure() const
	{
		return (uint32_t(this->segment) << 16) | this->offset;
	}
} PACKED;
