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



global memset
global memset32
global memcmp
global memcpy





SECTION .text
BITS 32


memset:
	xor eax, eax
	mov al, [esp + 8]
	mov ecx, 0x01010101
	mul ecx
	mov [esp + 8], eax



memset32:

	push edi
	cld

	mov eax, [esp + 12]

	mov edi, [esp + 8]
	mov ecx, [esp + 16]
	mov edx, ecx
	shr ecx, 2
	jz .smol
	rep stosd

.smol:
	and edx, 3
	jz .ret
	stosb

	dec edx
	jz .ret
	shr eax, 8
	stosb

	dec edx
	jz .ret
	shr eax, 8
	stosb

.ret:
	pop edi
	mov eax, [esp + 4]
	retd




memcmp:
	push esi
	push edi

	mov ecx, [esp + 20]
	mov edi, [esp + 16]
	mov esi, [esp + 12]
	mov edx, ecx

	cld

	shr ecx, 2
	jz .smol
	repe cmpsd
	jne .after_cmp

.smol:
	mov ecx, edx
	and ecx, 3
	jz .after_cmp
	repe cmpsb

.after_cmp:
	mov ecx, 1
	mov edx, -1
	mov eax, 0
	cmovb eax, ecx
	cmova eax, edx

	pop edi
	pop esi
	ret



memcpy:
	push esi
	push edi

	mov edi, [esp + 12]
	mov esi, [esp + 16]
	mov ecx, [esp + 20]
	mov edx, ecx

	shr ecx, 2
	jz .smol
	rep movsd

.smol:
	mov ecx, edx
	and ecx, 3
	jz .ret
	rep movsb

.ret:
	pop edi
	pop esi
	mov eax, [esp + 4]
	ret
