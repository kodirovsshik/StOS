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



%define GPT_LOAD_ADDR 0x600



extern bootloader_main_wrapper
extern __bootloader_size_sectors
extern __bootloader_end
extern MBR_VERSION
extern heap_top



SECTION .boot
BITS 16


mbr:
	jmp .code
times 3 - ($ - $$) nop
times 11 - ($ - $$) db 0

.bpb:
times 79 db 0x00

.code:
	xor ax, ax

	mov ss, ax
	mov sp, 0x7C00

	sti
	cld

	mov ds, ax
	mov bx, es
	mov word [0x506], bx
	mov es, ax
	;mov gs, ax
	;mov fs, ax

	mov word [0x502], dx
	mov word [0x504], di

	mov ax, 0x0003
	int 0x10


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

	mov ebp, GPT_LOAD_ADDR
	push dword 0
	push dword 1
	push ebp
	push dword 0x00010010
	call read_lba
	add sp, 16

%if 1
	cmp dword [bp], 'EFI '
	jne gpt_err
	;cmp dword [bp + 4], 'PART'
	;jne gpt_err
%else
	mov si, GPT_LOAD_ADDR
	mov di, data.efi_signature
	mov cx, 6
	repe cmpsw
%endif

	mov ebx, [bp + 80] ;//GPT table entries count
	xor edi, edi ;//Partition counter
	;mov edx, 1 ;//LBA counter

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
	;mov eax, edx
	;stosd

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
	je gpt_err
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



;//stack cleaned by caller:
;//DL = disk
;//qword LBA
;//dword Soff16 destenation
;//word count
;//word 16
read_lba:
	;//mov [.backup], si
	mov si, sp
	times 2 inc si

	mov ah, 0x42
	clc
	push bp
	int 0x13
	pop bp
	jc read_err

	;//mov si, [.backup]
	ret
.backup:
dw 0



cpu_err:
	mov si, data.msg_cpu - mbr + 0x7C00
	call puts16
	jmp halt

gpt_err:
	mov si, data.msg_no_gpt - mbr + 0x7C00
	call puts16
	jmp halt

read_err:
	mov si, data.msg_read_err - mbr + 0x7C00
	call puts16
	;//jmp halt



halt:
	hlt
	jmp halt



%define PUTC16_SMOL
%include "puts16.inc"





data:

.msg_read_err:
db "Read error", 0

.msg_cpu:
db "Incompatible CPU", 0

.msg_no_gpt:
db "No GPT found", 0

.msg_no_loader:
db "No loader found", 0

.stos_bootloader:
db "StOS bootloader "





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
