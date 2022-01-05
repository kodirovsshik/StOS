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
extern INTEL_CPUID_ALGORITHM

extern heap_top
extern heap_limit
extern __vbr_end

extern SEG_CODE16
extern SEG_CODE32
extern SEG_DATA16
extern SEG_DATA32
extern STACK_SIZE



%include "vbr.inc"



SECTION .rodata

gdt:
.null:
dq 0
.code32:
dw 0xFFFF
db 0x00, 0x00, 0x00
db 0b10011000, 0b11001111
db 0x00
.data32:
dw 0xFFFF
db 0x00, 0x00, 0x00
db 0b10010010, 0b11001111
db 0x00
.code16:
dw 0xFFFF
db 0x00, 0x00, 0x00
db 0b10011000, 0b00000000
db 0x00
.data16:
dw 0xFFFF
db 0x00, 0x00, 0x00
db 0b10010010, 0b00000000
db 0x00
.end:

gdt_descriptor:
dw gdt.end - gdt - 1
dd gdt


global gdt.code16
global gdt.data16
global gdt.code32
global gdt.data32





SECTION .text
BITS 16



err:
.return_cpu:
	mov eax, 0x80000003
	retd

.cpu:
	cmp IS_STOS_REQ, 0
	jne .return_cpu
	mov bp, msg_incompatible_cpu
	mov cx, msg_incompatible_cpu.end
	sub cx, bp
	dec cx
	;//jmp .handle_errors

.handle_errors:
	xor ax, ax
	mov ss, ax
	mov sp, 0x7C00

	mov ax, 0x1301
	mov bx, 0x000F
	mov dx, 0
	sti
	int 0x10
	;//jmp halt



halt:
	hlt
	jmp halt



stage2:
	call INTEL_CPUID_ALGORITHM
	jc err.cpu

	mov bp, sp

	;//eax = memsize

	xor eax, eax
	int 0x12
	mov ebx, 639
	cmp eax, 640
	cmove eax, ebx
	shl eax, 10

	lgdt [gdt_descriptor]

	cli

	mov ebx, cr0
	or bl, 1
	mov cr0, ebx

	mov bx, SEG_DATA32
	mov ds, bx
	mov ss, bx
	mov es, bx
	mov fs, bx
	mov gs, bx
	jmp SEG_CODE32:.32bit
.32bit:
BITS 32

	cmp IS_STOS_REQ, 0
	je .heap_setup_unknown

.heap_setup_stos:
	;//heap limit = (heap top < 0x7C00 ? 0x7C00 : mem size)
	mov bx, CX_BACKUP
	mov ebx, dword [bx + 8]
	mov dword [heap_top], ebx

	mov ecx, 0x7C00
	cmp ebx, ecx
	cmovb eax, ecx
	mov dword [heap_limit], eax
	jmp .heap_done

.heap_setup_unknown:
%if 0
	B = 0x7C00 - stack size - 0x600
	C = mem size - vbr end
	if B > C:
		heap top = 0x600
		heap limit = 0x7C00 - stack size
	else:
		heap top = vbr end
		heap limit = mem size
%endif
	mov ebx, 0x7C00 - 0x600 - STACK_SIZE
	mov ecx, edi
	sub ecx, __vbr_end
	cmp ebx, ecx

	mov ebx, __vbr_end
	mov ecx, edi
	mov esi, 0x600
	mov edi, 0x7C00 - STACK_SIZE
	cmovae esi, ebx
	cmovae edi, ecx
	mov dword [heap_top], esi
	mov dword [heap_limit], edi
	;//jmp .heap_done

.heap_done:

	cmp IS_STOS_REQ, 0
	jne .load_req

	push dword 0
	call dword main
	jmp halt

.load_req:
	xor ecx, ecx
	mov cx, CX_BACKUP
	push ecx
	call dword main
	add esp, 4

	mov ebx, SEG_DATA16
	mov ds, ebx
	mov ss, ebx
	mov es, ebx
	mov fs, ebx
	mov gs, ebx
	jmp SEG_CODE16:.16bit
.16bit:
BITS 16
	mov ebx, cr0
	and bl, 0xFE
	mov cr0, ebx

	xor ebx, ebx
	mov ds, bx
	mov ss, bx
	mov es, bx
	mov fs, bx
	mov gs, bx
	jmp 0x0000:.fix_cs
.fix_cs:
	retd



BITS 32
get_boot_disk:
	xor eax, eax
	mov al, DL_BACKUP
	retd



get_partition_table_entry:
	mov eax, DS_SI_BACKUP_AREA
	retd





SECTION .data
;//no

;//lmao tf is this line just above
;//If only you saw me when it had ruined my build and i got here to see what caused a build error
;//(there was no semicolon to indicate an asm comment)



SECTION .rodata

msg_incompatible_cpu:
db "Error: incompatible CPU", 0
.end:
