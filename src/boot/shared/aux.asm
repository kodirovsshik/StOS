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



global memcpy
global memset
global memcmp
global strncmp


BITS 16
SECTION .text



;//cdecl
;//dword size
;//dword byte value
;//dword ptr
memset:
	xor eax, eax
	mov edx, eax
	mov al, [esp + 8]
	mov ecx, 0x01010101
	mul ecx
	mov [esp + 8], eax



;//cdecl
;//dword size
;//dword byte value
;//dword ptr
memset32:
	push edi

	cld
	mov di, word [esp + 8]
	mov eax, dword [esp + 12]
	mov cx, word [esp + 16]

	mov dx, cx
	shr cx, 4

	rep stosd

	test dx, dx
	jz .ret

	stosb
	dec dx
	jz .ret

	shr eax, 8
	stosb
	dec dx
	jz .ret

	mov [di], ah
.ret:
	pop edi
	mov eax, dword [esp + 4]
	retd



;//cdecl
;//dword size
;//dword src
;//dword dst
memcpy:
	push esi
	push edi

	cld
	mov esi, dword [esp + 16]
	mov edi, dword [esp + 12]
	mov ecx, dword [esp + 20]

	mov eax, edi ;//we return dst

	mov edx, ecx
	shr ecx, 2
	rep movsd

	mov cx, dx
	and cx, 3
	rep movsb

	pop esi
	pop edi
	retd



;//cdecl
;//dword size
;//dword s2
;//dword s1
memcmp:
strncmp:
	push si
	push di

	xor si, si
	mov di, si
	mov es, di

	cld
	mov si, word [esp +  8]
	mov di, word [esp + 12]
	mov cx, word [esp + 16]
	repe cmpsb


	mov eax, 1
	mov ecx, -1
	je .zero
	cmovb eax, ecx
	jmp .ret

.zero:
	xor eax, eax
.ret:
	pop di
	pop si
	retd
