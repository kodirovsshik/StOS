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



global inb
global inw
global ind

global outb
global outw
global outd



SECTION .text
BITS 32


inb:
	mov dx, [esp + 4]
	xor eax, eax
	in al, dx
	retd

inw:
	mov dx, [esp + 4]
	xor eax, eax
	in ax, dx
	retd

ind:
	mov dx, [esp + 4]
	in eax, dx
	retd


outb:
	mov dx, [esp + 4]
	mov al, [esp + 8]
	out dx, ax
	retd

outw:
	mov dx, [esp + 4]
	mov ax, [esp + 8]
	out dx, ax
	retd

outd:
	mov dx, [esp + 4]
	mov eax, [esp + 8]
	out dx, eax
	retd
