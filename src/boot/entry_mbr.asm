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
%define ADDR_RELOCATION_DELTA (0xFE00 - $$)


%ifdef _VBR_MODE
	%define VBR_MODE 1
%else
	%define VBR_MODE 0
%endif
%ifdef _MBR_MODE
	%define MBR_MODE 1
%else
	%define MBR_MODE 0
%endif

%if VBR_MODE == MBR_MODE
	%error MBR or VBR mode ?
%endif



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

	xor ax, ax
	mov ss, ax
	mov sp, 0x508

	push di
	push es
	push dx

	mov sp, __STACK_TOP
	sti

	cld

	mov si, data.cmos_registers
	mov di, DATA_CMOS_ADDRESSES_AREA
	mov cx, 7
.cmos_loop:
	lodsb
	out 0x70, al
	in al, 0x71
	stosb
	loop .cmos_loop

%if VBR_MODE
	mov es, ax
	mov di, DATA_DS_SI_CONTENTS_AREA
	mov cx, 8
	rep movsw
%else
	mov di, DATA_LBA_PACKET_AREA
%endif

	mov ax, 16
	stosw
	mov ax, __bootloader_size_sectors
	stosw
	mov ax, 0x600
	stosw
	xor ax, ax
	stosw

%if VBR_MODE
	mov si, DATA_DS_SI_CONTENTS_AREA
	times 2 movsw
	xor ax, ax
%else
	inc ax
	stosw
	dec ax
	stosw
%endif
	times 2 stosw

	xor ax, ax
	mov ds, ax

%if VBR_MODE
	inc al
%endif
	mov DATA_IS_IN_VBR_MODE, al

	mov si, DATA_LBA_PACKET_AREA
	mov ah, 0x42
	int 0x13
	jc read_err
	test ah, ah
	jnz read_err

	jmp 0x0000:bootloader_main_wrapper



read_err:
	mov cx, data.msg_read_err_end - data.msg_read_err
	mov si, data.msg_read_err
	;//jmp print_err



;//DF clear
;//CX = count
;//DS:SI = string ptr
print_err:
	mov ah, 0x0E
	mov bx, 0x0007
.loop:
	lodsb
	int 0x10
	loop .loop
.halt:
	hlt
	jmp .halt



data:

.cmos_registers:
db 0, 2, 4, 6, 7, 8, 9

.msg_read_err:
db "Read error"
.msg_read_err_end:



ALIGN 2
mbr_data:

.align:
times 512 - 2 - 64 - ($ - $$) db 0xCC

.partition_table:
times 64 db 0

.signature:
db 0x55, 0xAA



end:
