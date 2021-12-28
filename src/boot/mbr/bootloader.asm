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
global _sleep_ns_unchecked
global _invoke_vbr_helper

extern main
extern current_boot_drive
extern puts
extern putc



%define EOI 0x20





;//SECTION .data

;//sleep_finished:
;//db 0





SECTION .rodata

msg_halted:
db 10, "Execution halted", 0





BITS 16
SECTION .text



bootloader_main_wrapper:
	sti
	;//mov dword [32], timer_handler

	mov dx, word [0x502]
	mov byte [current_boot_drive], dl

	call dword main
	int 0x18



halt:
	push dword msg_halted
	call dword puts
.jmp:
	hlt
	jmp .jmp



;//return _invoke_vbr_helper(hdr, disk, &entry);
;//cdecl
_invoke_vbr_helper:
	push ebp
	mov ebp, esp
	pushad

	xor si, si
	mov ds, si
	mov si, word [bp + 8]
	mov bx, .msg
	mov dl, byte [bp + 12]
	mov dh, byte [0x503]
	mov di, word [0x506]
	mov es, di
	mov di, word [0x504]
	mov ecx, dword [bp + 16]

	call dword 0x7C00

	xor bx, bx
	mov es, bx
	mov ds, bx

	mov dword [.tmp], eax
	popad

	mov eax, dword [.tmp]
	pop ebp
	retd

.msg:
db "StOSrequ", 0
.tmp:
dd 0


%if 0
timer_handler:
	push ax

	mov byte [sleep_finished], 1

	mov al, EOI
	out 0x20, al

	pop ax
	iret



;//cdecl
;//dword uint16_t count
_sleep_ticks:
	pushfd
	cli

	mov byte [sleep_finished], 0

	mov al, 0b00110000
	out 0x43, al
	jmp $+2

	mov ax, word [esp + 8]
	out 0x40, al
	jmp $+2

	mov al, ah
	out 0x40, al
	jmp $+2

	sti
.wait:
	cmp byte [sleep_finished], 0
	jne .ret
	hlt ;//We will resume after INT 8
	jmp .wait

.ret:
	popfd
	retd



;//cdecl
;//dword ns
_sleep_ns_unchecked:
	mov eax, dword [esp + 4]
	xor edx, edx
	mov ecx, 1193181
	mul ecx
	mov ecx, 1000000000
	div ecx
	inc eax
	sub edx, 500000000
	sbb eax, 0
	push eax
	call dword _sleep_ticks
	add esp, 4
	retd
%endif



_mbr_return:
	int 0x18
	jmp halt



;//dword ds:si ptr
_mbr_transfer_control_flow:
	xor esi, esi
	xor edi, edi
	xor ebp, ebp
	xor edx, edx

	mov eax, dword [esp + 4]
	mov si, ax
	mov bp, ax

	xor ax, ax
	mov ds, ax

	mov dx, word [0x502]
	mov di, word [0x504]
	mov ax, word [0x506]
	mov es, ax

	cli
	xor ax, ax
	mov sp, 0x7C00
	mov ss, ax
	sti

%if 1
	test si, si
	jz .skip_vbr ;//If we load a VBR

	test dl, dl
	jns .skip_hdd ;//From an HDD

	mov byte [si], dl
	;//Save drive number to MBR partinion table entry's active field
	;//Some old VBRs expect to find drive index in the VBR
	;// instead of finding 0x80

.skip_vbr:
	mov cx, 4
	mov bx, 0x7DBE

.loop:
	mov al, byte [bx]
	test al, al
	cmovs ax, dx
	mov byte [bx], al
	add bx, 16
	loop .loop
%endif

.skip_hdd:
	xor eax, eax
	xor ebx, ebx
	xor ecx, ecx

	jmp 0x0000:0x7C00
