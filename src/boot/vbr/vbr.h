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



#ifndef _VBR_H_
#define _VBR_H_

#include "defs.h"
#include "mbr.h"



_EXTERN_C_

[[noreturn]]
void halt();


mbr_entry_t* get_partition_table_entry();


[[noreturn]]
void native_boot();


uint32_t rh_get_boot_options(stos_request_header_t*);
uint32_t rh_boot            (stos_request_header_t*);

_EXTERN_C_END_



#endif //!_VBR_H_
