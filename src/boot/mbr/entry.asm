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



%define MBR_RELOC 0xFE00




SECTION .boot_sector_text
BITS 16


mbr:
	jmp .code
	times 3 - ($ - $$) nop

.bpb:
times 79 - ($ - mbr) db 0x00

.code:
	mov ax, 0x0003
	int 0x10

	xor ax, ax
	mov ds, ax
	mov bx, es
	mov word [0x506], bx
	mov es, ax
	mov gs, ax
	mov fs, ax

	mov word [0x502], dx
	mov word [0x504], di

	cli
	mov sp, ax
	mov ax, 0x7000
	mov ss, ax
	sti


	mov di, MBR_RELOC
	mov si, 0x7C00
	mov cx, 256
	rep movsw

	jmp 0x0000 : .relocated - mbr + MBR_RELOC

.relocated:

	mov ax, 0x0200 + __bootloader_size_sectors
	mov bx, 0x0600
	mov cx, 0x0002
	xor dh, dh
	int 0x13
	jc read_err
	test ah, ah
	jnz read_err

%if 1
	;//Check for 8086/80186
	push sp
	pop ax
	xor ax, sp
	jnz cpu_err
	;//Got at least 80286

	pushf
	pop ax
	or ax, 0xF000
	push ax
	popf

	pushf
	pop ax
	test ax, 0xF000
	jz cpu_err
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
	jz cpu_err
	;//Got at least 80486

	mov eax, ecx
	xor eax, 1 << 21
	push eax
	popfd

	pushfd
	pop eax
	xor eax, ecx
	jz cpu_err
	;//Got CPUID


	;//Restore a nice value for EFLAGS
	push dword (1 << 9)
	popfd
%endif

	xor eax, eax
	cpuid
	test eax, eax
	jz cpu_err
	;//Got leaf 1

	mov eax, 1
	cpuid
	test edx, 1 << 15 ;//test for CMOV
	jz cpu_err
	;//Got CMOV

	int 0x12
	cli

	xor bx, bx
	mov ss, bx

	cmp ax, 64

	mov sp, bx
	mov ax, __bootloader_end + 2048
	cmovb sp, ax

	mov dx, __bootloader_end
	cmovae ax, dx

	movzx eax, ax
	mov dword [heap_top], eax

	jmp 0x0000 : bootloader_main_wrapper


cpu_err:
	mov si, data.msg_cpu_err - mbr + MBR_RELOC
	call puts
	jmp halt

read_err:
	mov si, data.msg_read_err - mbr + MBR_RELOC
	call puts
	jmp halt

;//DS:SI = C string ptr
puts:
	push ax
	push bx
	mov ah, 0x0E
	mov bx, 0x0007
.loop:
	mov al, byte [si]

	test al, al
	jz .end

	int 0x10
	inc si

	cmp al, 10
	jne .loop

	mov al, 13
	int 0x10
	jmp .loop
.end:
	pop bx
	pop ax
	ret



halt:
	hlt
	jmp halt





data:

.msg_read_err:
db "Disk read error", 0

.msg_cpu_err:
db "Error: 32 bit CPU with CMOV is required", 0





metadata:

.align:
times 0x1AD - ($ - mbr) db 0xCC

.signature:
db "StOSboot"

.version:
dw MBR_VERSION

.size:
db __bootloader_size_sectors

.disk_signature:
times 6 db 0x00

.mbr_partition_table:
times 64 db 0x00

.mbr_signature:
db 0x55, 0xAA
