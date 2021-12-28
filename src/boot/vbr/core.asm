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



global msg_incompatible_cpu
global msg_incompatible_cpu.end
global gdt
global stage2



extern main



%include "vbr.inc"



SECTION .text
BITS 16

stage2:
	cmp IS_STOS_REQ, 0
	jne .load_req

	push dword 0
	call main
	jmp halt

.load_req:
	xor ecx, ecx
	mov cx, CX_BACKUP
	push ecx
	call main
	add esp, 4
	retd



halt:
	hlt
	jmp halt



get_boot_drive:
	xor eax, eax
	mov al, DL_BACKUP
	retd



get_partition_table_entry:
	mov eax, DS_SI_BACKUP_AREA
	retd



go_unreal:
;//assume real mode already
	cli
	lgdt [gdt]
	jmp 0x0000:.jmp
.jmp:
	mov eax, cr0
	or ax, 1
	mov cr0, eax

	mov cx, GDT_DATA
	mov ds, cx
	mov ss, cx
	mov es, cx

	and ax, 0xFFFE
	mov cr0, eax
	retd



go_real:
	;//assume unreal mode already
	mov eax, cr0
	or ax, 1
	mov cr0, eax

	xor cx, cx
	mov ds, cx
	mov ss, cx
	mov es, cx
	mov gs, cx
	mov fs, cx
	jmp 0x0000:.jmp

.jmp:
	and ax, 0xFFFE
	mov cr0, eax

	sti
	retd





SECTION .data
;//no
;//lmao tf is this line just above
;//If only you saw me when it had ruined my build and i got here to see what caused a build error
;//(there was no ; to indicate asm comment)



SECTION .rodata

msg_incompatible_cpu:
db "Error: incompatible CPU", 0
.end:



gdt:

.null:
GDT_NULL equ 0
dq 0

;//TODO: make code segment 32 bit
.code:
times 2 db 0xFF
times 3 db 0
db 0b10011000
db 0b11001111
db 0x00
;//GDT_CODE equ gdt.code - gdt

.data:
times 2 db 0xFF
times 3 db 0
db 0b10010010
db 0b11001111
db 0x00
GDT_DATA equ gdt.data - gdt

.end:


gdt_descriptor:
dw gdt.end - gdt - 1
dd gdt
