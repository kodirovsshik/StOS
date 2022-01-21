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



;//global storage memory map

%define DATA_IS_IN_VBR_MODE           byte  [0x501]
%define DATA_DX_BACKUP                word  [0x502]
	%define DATA_DL_BACKUP              byte  [0x502]
	%define DATA_DH_BACKUP              byte  [0x503]
%define DATA_ES_BACKUP                word  [0x504]
%define DATA_DI_BACKUP                word  [0x506]
%define DATA_DS_SI_CONTENTS_AREA      0x508
%define DATA_LBA_PACKET_AREA          0x518
%define DATA_CMOS_CONTENTS_AREA       0x528
%define DATA_BOOTSTRAP_CODE_LBA_AREA  0x530
;//next: 0x538


%define ADDR_LOAD_DELTA (0x7C00 - $$)
%define ADDR_RELOCATION_DELTA (ADDR_RELOC - $$)
