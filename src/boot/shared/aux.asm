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


BITS 32
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
;//dword value
;//dword ptr
memset32:
	push edi

	cld

	mov ecx, dword [esp + 16]
	mov edx, ecx
	shr ecx, 2
	jz .ret

	mov edi, dword [esp + 8]
	mov eax, dword [esp + 12]
	rep stosd

	mov ecx, edx
	and ecx, 3
	jz .ret
	rep stosb

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

	mov ecx, dword [esp + 20]
	mov edx, ecx
	shr ecx, 2
	jz .ret

	mov esi, dword [esp + 16]
	mov edi, dword [esp + 12]
	mov eax, edi ;//we return dst
	rep movsd

	mov ecx, edx
	and ecx, 3
	jz .ret
	rep movsb

.ret:
	pop esi
	pop edi
	retd



;//cdecl
;//dword size
;//dword s2
;//dword s1
memcmp:
strncmp:
	push esi
	push edi

	cld
	mov esi, dword [esp + 12]
	mov edi, dword [esp + 16]
	mov ecx, dword [esp + 20]
	repe cmpsb

	mov eax, 1
	mov ecx, -1
	je .zero
	cmovb eax, ecx
	jmp .ret

.zero:
	xor eax, eax
.ret:
	pop edi
	pop esi
	retd
