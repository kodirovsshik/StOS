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
global interrupt_smol

extern SEG_CODE16
extern SEG_CODE32
extern SEG_DATA16
extern SEG_DATA32


;//Flags that are allowed to be set via interrupt() call
;//Excludes: Reserved, IOPL, IF, TF, NT, everything outside FLAGS register
FLAGS_MASK equ 0b110011010101



BITS 32
SECTION .text


;//stdcall:
;//dword uint8_t N
;//eax, ebx, ecx, edx, esi, edi, ebp, eflags are forwarded to and from interrupt as is
interrupt_smol:
	pushfd
	push ebp
	push edi
	push esi
	push ebx
	push edx
	push ecx
	push eax

	push esp
	push dword [esp + 40]
	call dword interrupt
	add esp, 8

	pop eax
	pop ecx
	pop edx
	pop ebx
	pop esi
	pop edi
	pop ebp
	popfd
	retd 4



;//cdecl:
;//dword void* regs
;//dword uint8_t N
interrupt:
	pushad
	;push ebp
	mov ebp, esp
	mov dword [.save_area], ebp
	sub esp, 6*4

	cld
	mov edi, esp

	mov eax, cs
	stosd
	mov eax, ds
	stosd
	mov eax, ss
	stosd
	mov eax, es
	stosd
	mov eax, fs
	stosd
	mov eax, gs
	stosd

	mov eax, SEG_DATA16
	mov ds, eax
	mov ss, eax
	mov es, eax
	mov gs, eax
	mov fs, eax
	jmp SEG_CODE16:.16bit
BITS 16
.16bit:
	mov eax, cr0
	and al, 0xFE
	mov cr0, eax

	xor ax, ax
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	jmp 0x0000:.fix_cs
.fix_cs:
	mov ax, word [ebp + 36]
	mov byte [.interrupt_number], al

	mov eax, dword [ebp + 40]
	push dword [eax + 28]
	pushfd
	pop ebx
	and dword [esp], FLAGS_MASK
	and ebx, ~FLAGS_MASK
	or dword [esp], ebx
	mov ebp, dword [eax + 24]
	mov edi, dword [eax + 20]
	mov esi, dword [eax + 16]
	mov ebx, dword [eax + 12]
	mov edx, dword [eax + 8]
	mov ecx, dword [eax + 4]
	mov eax, dword [eax + 0]

	popfd
	sti

	db 0xCD
.interrupt_number:
	db 0x00

	cli
	pushfd
	push ebp
	push edi
	push esi
	push ebx
	push edx
	push ecx
	push eax

	mov ebp, dword [.save_area]

	mov eax, cr0
	or al, 1
	mov cr0, eax

	cld
	lea esi, [ebp - 6*4]

	lodsd
	push ax
	push word .32bit
	retf
BITS 32
.32bit:
	lodsd
	mov ds, eax
	lodsd
	mov ss, eax
	lodsd
	mov es, eax
	lodsd
	mov fs, eax
	lodsd
	mov gs, eax

	mov esi, esp
	mov edi, [ebp + 32 + 4 + 4]
	mov ecx, 8
	rep movsd

	mov esp, ebp
	popad

	retd

ALIGN 4
.save_area:
dd 0
