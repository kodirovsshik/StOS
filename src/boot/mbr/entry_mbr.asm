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



extern bootloader_main_wrapper
extern __bootloader_size_sectors
extern __bootloader_end
extern MBR_VERSION
extern heap_top



SECTION .boot_sector_text
BITS 16


mbr:
	jmp .code
times 3 - ($ - $$) nop
times 11 - ($ - $$) db 0

.bpb:
times 79 db 0x00

.code:
	cli

	xor ax, ax
	mov ds, ax
	mov bx, es
	mov word [0x506], bx
	mov es, ax
	mov gs, ax
	mov fs, ax

	mov word [0x502], dx
	mov word [0x504], di

	mov ss, ax
	mov sp, 0x7C00

	sti
	mov ax, 0x0003
	int 0x10

	mov ax, 0x0200 + __bootloader_size_sectors
	mov bx, 0x0600
	mov cx, 0x0002
	xor dh, dh
	int 0x13
	jc read_err

	test ah, ah
	jnz read_err

	jmp 0x0000 : bootloader_main_wrapper



read_err:
	mov si, data.msg_read_err - mbr + 0x7C00
	call puts16
	;//jmp halt



halt:
	hlt
	jmp halt



%include "puts16.inc"



data:

.msg_read_err:
db "Disk read error", 0






metadata:

.align:
times 0x1AD - ($ - mbr) db 0xCC

.signature:
db "StOSboot"

.version:
dw MBR_VERSION

.size:
db __bootloader_size_sectors



mbr_metadata:

.disk_signature:
times 6 db 0x00

.mbr_partition_table:
times 64 db 0x00

.mbr_signature:
db 0x55, 0xAA

end:
