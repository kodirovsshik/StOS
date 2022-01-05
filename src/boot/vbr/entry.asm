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



extern VBR_VERSION
extern __vbr_size_sectors
extern gdt_descriptor
extern gdt
extern msg_incompatible_cpu
extern msg_incompatible_cpu.end
extern stage2


global entry


SECTION .boot
BITS 16


%define DS_BACKUP word [scratch_area.ds]
%define SI_BACKUP word [scratch_area.si]

%include "./vbr.inc"



entry:
	jmp code
	nop

times 3 - ($ - entry) db 0xCC
times 11 - ($ - entry) db 0x00


bpb:
times 90 - ($ - entry) db 0x00

code:
	cli

	mov ax, ds

	xor bp, bp
	mov ds, bp

	mov DX_BACKUP, dx
	mov CX_BACKUP, cx
	mov DS_BACKUP, ax
	mov SI_BACKUP, si

	mov [ES_DI_BACKUP_AREA], di
	mov ax, es
	mov [ES_DI_BACKUP_AREA + 2], ax

	xor di, di
	mov es, di

	mov di, DS_SI_BACKUP_AREA
	mov cx, 8
	rep movsw

	mov di, 0x5A8
	mov si, data.cmos_ports

	mov cx, 7
	cld

.cmos_loop:
	lodsb
	out 0x70, al
	in al, 0x71
	stosb
	loop .cmos_loop

	mov al, 0
	stosb


	mov si, data.stosrequ
	mov di, bx
	mov cx, 8
	repe cmpsb

	;//cmov goes brrrrrr
	mov IS_STOS_REQ, 1
	je .L1
	mov IS_STOS_REQ, 0
.L1:

	mov di, LBA_PACKET_AREA

	;//Byte struct size and reserved
	mov ax, 0x0010
	stosw

	;//number of sectors to read
	mov ax, __vbr_size_sectors
	stosw

	;//offset
	mov ax, 0x0000
	stosw

	;//segment
	mov ax, 0x07C0
	stosw

	;//LBA bytes 0-3
	mov si, DS_SI_BACKUP_AREA + 8
	movsw
	movsw

	cmp IS_STOS_REQ, 0
	je .native_boot

.request:
	movsw
	movsw
	call aux.int13read
	jc .err_return_read
	test ah, ah
	jnz .err_return_read
	jmp .after_read

.native_boot:
	xor ax, ax
	stosw
	stosw

	mov ss, ax
	mov sp, 0x7C00

	call aux.int13read
	jc .err_read
	test ah, ah
	jnz .err_read
	;jmp .after_read

.after_read:

	jmp 0x0000:stage2

.err_return_read:
	mov eax, 0x80000002
	retd

.err_read:
	mov cx, data.err_read_end - data.err_read
	mov bp, data.err_read
	;jmp error



error:
	mov ax, 0x1301
	mov bx, 0x000F
	mov dx, 0
	int 0x10
	;jmp halt



halt:
	hlt
	jmp halt






aux:
.int13read:
	mov ah, 0x42
	mov si, LBA_PACKET_AREA
	clc
	sti
	int 0x13
	ret





data:

.err_read:
db "Read error"
.err_read_end:

.cmos_ports:
db 0, 2, 4, 6, 7, 8, 9

.stosrequ:
db "StOSrequ"



metadata:

.align:
times 0x1AD - ($ - entry) db 0xCC

.signature:
db "StOSload"

.version:
dw VBR_VERSION

.size:
db __vbr_size_sectors


scratch_area:
.ds: dw 0
.si: dw 0


bios_data:

.align:
times 446 - ($ - entry) db 0xCC

;//even tho i'm making a VBR, it's probably better to leave some place for it just in case
.partition_table:
times 4*16 db 0x00

.signature:
db 0x55, 0xAA
