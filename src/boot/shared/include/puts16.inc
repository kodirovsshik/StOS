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



;//DS:SI = C string ptr
puts16:

%ifdef PUTC16_SMOL

	mov ah, 0x0E
	mov bx, 0x0007
.loop:
	lodsb
	int 0x10
	test al, al
	jnz .loop
	ret

%else

	push ax
	push bx
	mov ah, 0x0E
	mov bx, 0x0007
	cld
	test si, si
	jnz .loop
	mov si, .msg_null
.loop:
	lodsb
	test al, al
	jz .end

	int 0x10

	cmp al, 10
	jne .loop

	mov al, 13
	int 0x10
	jmp .loop
.end:
	pop bx
	pop ax
	ret
.msg_null:
db "(null)", 0

%endif
