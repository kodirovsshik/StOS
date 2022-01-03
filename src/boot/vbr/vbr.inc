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


%define DS_SI_BACKUP_AREA 0x580
%define ES_DI_BACKUP_AREA 0x590
%define DX_BACKUP word [0x594]
%define DL_BACKUP byte [0x594]
%define DH_BACKUP byte [0x595]
%define LBA_PACKET_AREA 0x596
%define CX_BACKUP word [0x5A6]
%define INVOKE_DATETIME_AREA 0x5A8
%define IS_STOS_REQ byte [0x5A9]


%define STACK_SIZE 2048
