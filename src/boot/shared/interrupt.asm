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



global interrupt


BITS 16
SECTION .text


;//cdecl
;//stack:
;//dword uint8_t N
;//dword void* regs
interrupt:
	pushad

	mov al, byte [esp + 40]
	mov byte [.num], al

	mov eax, dword [esp + 36]
	mov ebp, dword [eax + 24]
	mov edi, dword [eax + 20]
	mov esi, dword [eax + 16]
	mov ebx, dword [eax + 12]
	mov edx, dword [eax + 8]
	mov ecx, dword [eax + 4]
	mov eax, dword [eax]

	db 0xCD
.num:
	db 0x00

	push eax

	mov ax, 0 ;//don't modify flags
	mov ds, ax
	mov es, ax

	mov eax, dword [esp + 40]
	mov dword [eax + 4], ecx
	mov dword [eax + 8], edx
	mov dword [eax + 12], ebx
	mov dword [eax + 16], esi
	mov dword [eax + 20], edi
	mov dword [eax + 24], ebp
	pop ecx
	mov dword [eax], ecx
	pushfd
	pop ebx
	mov dword [eax + 28], ebx

	popad
	retd
