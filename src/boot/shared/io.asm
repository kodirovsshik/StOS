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



%include "int16.inc"



global puts
global putc
global endl
global _drive_lba_helper
global drive_lba_supported
global get_drives_count
global getch
global kbhit
global select_video_page



BITS 32
SECTION .text



;//cdecl
;//dword string ptr
puts:
	push ebx
	mov edx, [esp + 8]
	mov ah, 0x0E
	mov bx, 0x0007
.loop:
	mov al, byte [edx]
	test al, al
	jz .ret
	int16 0x10
	inc edx
	cmp al, 10
	jne .loop
	mov al, 13
	int16 0x10
	jmp .loop
.ret:
	pop ebx
	retd



;//cdecl
;//dword char
putc:
	push bx
	mov ah, 0x0E
	mov bx, 0x0007
	mov al, byte [esp + 6]
	int16 0x10
	cmp al, 10
	jne .ret
	mov al, 13
	int16 0x10
.ret:
	pop bx
	retd



endl:
	push bx
	mov ax, 0x0E0D
	mov bx, 0x0007
	int16 0x10
	mov al, 0x0A
	int16 0x10
	pop bx
	retd



getch:
	xor ah, ah
	int16 0x16
	retd



kbhit:
	mov ah, 1
	int16 0x16
	retd



get_drives_count:
	mov al, byte [0x475]
	retd



;//dword uint8_t
;//dword void*
_drive_lba_helper:
	push esi
	mov dl, byte [esp + 12]
	mov si, word [esp + 8]
	xor ax, ax
	xchg ah, byte [si + 1]
	int16 0x13
	shr ax, 8
	pop esi
	retd



;//dword uint8_t index
drive_lba_supported:
	push ebx

	mov ah, 0x41
	mov bx, 0x55AA
	mov dl, byte [esp + 8]
	int16 0x13
	mov al, 0
	jc .no

	cmp bx, 0xAA55
	jne .no

	test cx, 1
	jz .no

	mov al, 1
.no:
	pop ebx
	retd
