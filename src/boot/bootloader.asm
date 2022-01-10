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



global bootloader_main_wrapper
global halt
global _mbr_return
global _mbr_transfer_control_flow
global panic
global invoke_main

global global_canary
global default_canary

extern INTEL_CPUID_ALGORITHM
extern bootloader_init
extern main

extern heap_limit
extern heap_top

extern __STACK_TOP
extern SEG_CODE16
extern SEG_CODE32
extern SEG_DATA16
extern SEG_DATA32





SECTION .canary

global_canary:
times 16 db 0





SECTION .rodata

gdt:
.null:
dq 0
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
.end:

gdt_descriptor:
dw gdt.end - gdt - 1
dd gdt


global gdt
global gdt.code16
global gdt.code32
global gdt.data16
global gdt.data32



str_posterror:
db "Press any key to try booting from another device or Ctrl-Alt-Del to reboot", 0

str_error:
db "Error: ", 0

str_panic_bad:
db "(error message lies outside 64KiB boundary)", 0

str_cpu_error:
db "32 bit CPU with CMOV is required to run StOS loader", 0

str_memory_error:
db "Low amount of memory, 64 KiB of memory if required to run StOS loader", 0




default_canary:
db "CanaryStOSCanary"




SECTION .text
BITS 16


bootloader_main_wrapper:
	mov ax, 0x2401
	int 0x15

	xor ax, ax
	mov fs, ax
	mov gs, ax

	call INTEL_CPUID_ALGORITHM
	jc error.cpu

	mov si, default_canary
	mov di, global_canary
	mov cx, 4
	rep movsd

	cli

	xor ebp, ebp

	mov eax, cr0
	or al, 1
	mov cr0, eax

	lgdt [gdt_descriptor]

	mov ax, SEG_DATA32
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	jmp SEG_CODE32:bootloader_init



error:

.cpu:
	push str_cpu_error
	;//jmp .handler

.handler:
	sti

	mov ax, 0x0003
	int 0x10

	mov si, str_error
	call puts

	pop si
	call puts
	call endl

	mov si, str_posterror
	call puts

	call getch
	call endl
	int 0x18

	jmp halt



getch:
	xor ah, ah
	int 0x16
	xor ah, ah
	ret



halt:
	hlt
	jmp halt



;//DF clear
;//SI = string ptr
cls:
	mov ax, 3
	int 0x10
	ret



endl:
	mov ax, 0x0E0D
	mov bx, 0x0007
	int 0x10
	mov al, 10
	int 0x10
	ret



;//SI = C string ptr
puts:
	cld
	mov ah, 0x0E
	mov bx, 0x0007
.loop:
	lodsb
	test al, al
	jz .ret
	int 0x10
	cmp al, 10
	jne .loop
	mov al, 13
	int 0x10
	jmp .loop
.ret:
	ret



BITS 32
panic:
	mov ecx, dword [esp + 4]
	test ecx, 0xFFFF0000
	mov edx, str_panic_bad
	cmovnz ecx, edx

	push cx

	mov eax, SEG_DATA16
	mov ds, eax
	mov ss, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
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
	jmp 0x0000:error.handler



BITS 32
invoke_main:
	mov esp, __STACK_TOP
	xor ebp, ebp
	call main

BITS 16
