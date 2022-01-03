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



#ifndef _SMOL_VEC_HPP_
#define _SMOL_VEC_HPP_

#include "memory.h"



template<class T>
struct smol_vec
{
	T* data;
	size_t size;
	size_t capacity;

	static constexpr size_t capacity_step = 8;

	void init(size_t cap)
	{
		this->data = (T*)malloc(cap * sizeof(T));
		this->size = 0;
		this->capacity = cap;
	}

	void push_back(const T& x)
	{
		if (this->size == this->capacity)
		{
			realloc(this->data, this->capacity, this->capacity + capacity_step);
			this->capacity += capacity_step;
		}
		this->data[this->size++] = x;
	}

	T& operator[](size_t i)
	{
		return this->data[i];
	}

	void clear()
	{
		this->size = 0;
	}
	void deallocate()
	{
		free(this->data, this->capacity);
		this->size = this->capacity = 0;
	}
};

#endif //!_SMOL_VEC_HPP_
