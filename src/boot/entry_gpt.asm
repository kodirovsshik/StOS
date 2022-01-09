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



%include "include/mbr.inc"


%define ADDR_LOAD_DELTA (0x7C00 - $$)
%define GPT_LOAD_ADDR 0x600



global mbr

extern __bootloader_size_sectors
extern bootloader_main_wrapper
extern __STACK_TOP




SECTION .boot
BITS 16


mbr:
.entry:
	jmp .code
	times 3 - ($ - mbr) nop

.oem:
times 8 db 0

.bpb:
times 79 db 0


.code:

	cli

	xor cx, cx
	mov ss, cx
	mov sp, 0x508

	push di
	push es
	push dx

	mov sp, __STACK_TOP
	sti

	mov ds, cx

	cld

	mov si, data.cmos_registers + ADDR_LOAD_DELTA
	mov di, DATA_CMOS_ADDRESSES_AREA
	mov cx, 8
.cmos_loop:
	lodsb
	out 0x70, al
	in al, 0x71
	stosb
	loop .cmos_loop

	;//Check for 8086/80186
	push sp
	pop ax
	xor ax, sp
	jnz error.cpu
	;//Got at least 80286

	pushf
	pop ax
	or ax, 0xF000
	push ax
	popf

	pushf
	pop ax
	test ax, 0xF000
	jz error.cpu
	;//Got at least 80386 => got 32 bit instructions

	push dword 0x200
	popfd

	mov ebp, GPT_LOAD_ADDR
	push dword 0
	push dword 1
	push ebp
	push dword 0x00010010
	call read_lba
	add sp, 16

	cmp dword [bp], 'EFI '
	jne error.gpt
	cmp dword [bp + 4], 'PART'
	jne error.gpt


	mov ebx, [bp + 80] ;//GPT table entries count
	xor edi, edi ;//Partition counter

	xor eax, eax
	push eax
	inc ax
	push eax
	sub sp, 4
	push dword 0x00010010


	mov ebp, eax
;//outer loop: loops through LBAs
.gpt_loop1:
	mov bp, GPT_LOAD_ADDR

	lea edi, [esp + 4]
	mov ax, bp
	stosd
	inc dword [di]

	call read_lba

	mov ax, 4

;//inner loop: loops through GPT entries inside an LBA
.gpt_loop2:
	pusha

	mov si, bp
	mov di, data.stos_bootloader - mbr + 0x7C00
	mov cx, 8
	repe cmpsw

	popa

	je .gpt_found

	inc edi
	cmp edi, ebx
	je error.gpt
	add bp, 128
	dec ax
	jnz .gpt_loop2

	inc edx
	jmp .gpt_loop1

.gpt_found:
	;//bp = GPT entry ptr
	push dword [bp + 36]
	push dword [bp + 32]
	push dword 0x600
	push word __bootloader_size_sectors - 1
	push word 16
	call read_lba

	jmp 0x0000:bootloader_main_wrapper



read_lba:
	mov si, sp
	times 2 inc si

	mov ah, 0x42
	int 0x13
	jc error.read

	test ah, ah
	jnz error.read

	ret



error:
.cpu:
	mov cx, data.msg_cpu_err_end - data.msg_cpu_err
	mov si, data.msg_cpu_err + ADDR_LOAD_DELTA
	jmp .print
.read:
	mov cx, data.msg_read_err_end - data.msg_read_err
	mov si, data.msg_read_err + ADDR_LOAD_DELTA
	jmp .print
.gpt:
	mov cx, data.msg_gpt_err_end - data.msg_gpt_err
	mov si, data.msg_gpt_err + ADDR_LOAD_DELTA
	;jmp .print



;//DF clear
;//CX = count
;//DS:SI = string ptr
.print:
	mov ax, 3
	int 0x10
	mov ah, 0x0E
	mov bx, 0x0007
.loop:
	lodsb
	int 0x10
	loop .loop
.halt:
	xor ax, ax
	int 0x16
	int 0x18
	;jmp .halt



data:

.cmos_registers:
db 0, 2, 4, 6, 7, 8, 0x32, 9

.stos_bootloader:
db "StOS bootloader "

.msg_read_err:
db "Read error", 13
.msg_read_err_end:

.msg_cpu_err:
db "Outdated CPU", 13
.msg_cpu_err_end:

.msg_gpt_err:
db "Not a bootable GPT", 13
.msg_gpt_err_end:



mbr_data:

.align:
times 512 - 2 - 64 - 6 - ($ - $$) db 0xCC

.disk_signature:
times 6 db 0

.partition_table:
times 64 db 0

.signature:
db 0x55, 0xAA



end:
