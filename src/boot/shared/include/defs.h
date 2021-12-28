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



#ifndef _DEFS_H_
#define _DEFS_H_

#ifdef __cplusplus
	#define _EXTERN_C_ extern "C" {
	#define _EXTERN_C_END_ }
#else
	#define _EXTERN_C_
	#define _EXTERN_C_END_
#endif

#define PACKED __attribute__((packed))

#endif //!_DEFS_H_
