%if 0
	  This file is a part of StOS project - a small operating system
	  	made for learning purposes
	  This file's content is based on the CPU identification algorithm
	  	published by Intel in "Intel Processor Identification and the
			CPUID Instruction", AP-485
	  Copyright 1993, 1994, 1995, 1996 by Intel Corp
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



global INTEL_CPUID_ALGORITHM



BITS 16
SECTION .text



;//Sets carry if outdated CPU (no PM or CMOV) is detected
INTEL_CPUID_ALGORITHM:
	mov bp, sp
	push word 0
	pushf

	cli

	;//Check for 8086/80186
	push sp
	pop ax
	xor ax, sp
	jnz .err
	;//Got at least 80286

	pushf
	pop ax
	or ax, 0xF000
	push ax
	popf

	pushf
	pop ax
	test ax, 0xF000
	jz .err
	;//Got at least 80386

	pushfd
	pop eax
	mov ecx, eax
	xor eax, 1 << 18
	push eax
	popfd

	pushfd
	pop eax
	xor eax, ecx
	jz .err
	;//Got at least 80486

	mov eax, ecx
	xor eax, 1 << 21
	push eax
	popfd

	pushfd
	pop eax
	xor eax, ecx
	jz .err
	;//Got CPUID


	;//Restore original value for EFLAGS
	popf

	xor eax, eax
	cpuid
	test eax, eax
	jz .err
	;//Got leaf 1

	mov eax, 1
	cpuid
	test edx, 1 << 15 ;//test for CMOV
	jz .err
	;//Got CMOV

	clc
	jmp .ret
.err:
	stc
.ret:
	lea sp, [bp - 4]
	popf
	add sp, 2
	ret
