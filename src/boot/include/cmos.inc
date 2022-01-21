%if 0
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
%endif



%ifndef _CMOS_FETCH_INC_
%define _CMOS_FETCH_INC_

cmos_fetch:
	mov si, .regs_data + CURRENT_LOAD_DELTA
	mov di, DATA_CMOS_CONTENTS_AREA
	mov cx, 8
.loop:
	lodsb
	out 0x70, al
	in al, 0x71
	stosb
	loop .loop
.regs_data:
db 0, 2, 4, 6, 7, 8, 0x32, 9

%endif ;//!_CMOS_FETCH_INC_
