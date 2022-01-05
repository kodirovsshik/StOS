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
extern current_boot_disk
extern puts
extern putc
extern __bootloader_end
extern heap_top
extern heap_limit
extern INTEL_CPUID_ALGORITHM

extern SEG_CODE16
extern SEG_CODE32
extern SEG_DATA16
extern SEG_DATA32




%define EOI 0x20
%define STACK_SIZE 4096





;//SECTION .data

;//sleep_finished:
;//db 0





SECTION .rodata

msg_err_cpu:
db "Error: A 32 bit CPU with CMOV is required", 10, 0
msg_err_a20:
db "Error: Failed to activete A20 line", 10, 0
msg_err_memory:
db "Error: 64KiB of memory required", 10, 0
msg_halted:
db 10, "Execution halted", 0



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


global gdt
global gdt.code16
global gdt.code32
global gdt.data16
global gdt.data32





SECTION .text
BITS 16

%include "puts16.inc"



;//ZF set if A20 is disabled
check_a20:
	call delay16

	push ax
	push fs
	push gs
	push si
	push di

	xor ax, ax
	mov fs, ax
	not ax ;//0xFFFF
	mov gs, ax
	;//10057E <- X
	;//00057E <- Y

	mov si, 0x57E
	mov di, 0x58E ;//0x58E + 0xFFFF0 = 0x10057E

	mov word [fs:si], 0x1234
	mov word [gs:di], 0x5678
	mov ax, word [gs:di]
	sub ax, word [fs:si]

	pop di
	pop si
	pop gs
	pop fs
	pop ax
	ret



try_a20_kb:
	;//TODO: implement A20 enabling through kb
	ret



delay16:
	xor ecx, ecx
	dec cx
.loop:
	loop .loop
	ret



;//al = character
putc16:
	mov ah, 0x0E
	mov bx, 0x0007
	int 0x10
	cmp al, 10
	jne .ret
	mov al, 13
	int 0x10
.ret:
	ret



err_cpu:
	mov si, msg_err_cpu
	jmp preinit_error
err_a20_fail:
	mov si, msg_err_a20
	jmp preinit_error
err_memory_low:
	mov si, msg_err_memory
	;//jmp preinit_error
preinit_error:
	call puts16
	mov si, msg_halted
	call puts16
.halt:
	hlt
	jmp .halt


bootloader_main_wrapper:

	xor ax, ax
	mov ss, ax
	mov sp, 0x7C00
	call INTEL_CPUID_ALGORITHM
	jc err_cpu


%if 0
	//Memory map:
	//0x00600 - 0x0????: bootloader code
	//0x0???? - 0x0????: (possible heap space)
	//0x0???? - 0x07C00: stack
	//0x07C00 - 0x0????: VBR code
	//0x10000 - 0x?????: (possible heap space)

	//Available memory for heap between code and stack bottom
	A = 0x7C00 - __bootloader_end - STACK_SIZE
	//Available memory for heap after mbr and vbr code/stack
	B = mem size - 0x10000
	if A > B:
		heap top = __bootloader_end
		heap limit = 0x7C00 - STACK_SIZE
	else:
		heap top = 0x10000
		heap limit = memsize
%endif

	xor eax, eax
	int 0x12
	cmp eax, 64
	jb err_memory_low
	cmp eax, 640
	mov ebx, 639
	cmovae eax, ebx
	shl eax, 10
	;//eax is now memsize

	mov ebx, 0x7C00 - STACK_SIZE
	sub ebx, __bootloader_end
	lea ecx, [eax - 0x10000]

	cmp ebx, ecx

	mov ebx, __bootloader_end
	mov ecx, 0x7C00 - STACK_SIZE

	mov esi, 0x10000
	mov edi, eax

	cmovb ebx, esi
	cmovb ecx, edi

	mov dword [heap_top], ebx
	mov dword [heap_limit], ecx

	mov dx, word [0x502]
	mov byte [current_boot_disk], dl

.a20_check:
	call check_a20
	jnz .skip_a20

.a20_try_int15:
	mov ax, 0x2401
	int 0x15

	call check_a20
	jnz .skip_a20

.a20_try_kb:
	call try_a20_kb

	call check_a20
	jnz .skip_a20

.a20_try_ee:
	in al, 0xEE

	call check_a20
	jnz .skip_a20

.a20_try_ef:
	in al, 0xEF

	call check_a20
	jnz .skip_a20

.a20_try_92:
	in al, 0x92
	test al, 2
	jnz .a20_after_92
	or al, 2
	and al, 0xFE
	out 0x92, al

	call check_a20
	jnz .skip_a20

.a20_after_92:
	jmp err_a20_fail

.skip_a20:
	cli

	mov eax, cr0
	or ax, 1
	mov cr0, eax

	lgdt [gdt_descriptor]

	mov ax, SEG_DATA32
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov gs, ax
	mov fs, ax
	jmp SEG_CODE32:.32bits

BITS 32
.32bits:
	call dword main
	jmp _mbr_return



halt:
	push dword msg_halted
	call dword puts
.jmp:
	hlt
	jmp .jmp



;//dword ptr MBR entry ptr
;//dword uint8 disk number
;//dword ptr header ptr
;//cdecl
_invoke_vbr_helper:
	push ebx
	push esi
	push edi
	push ebp
	mov ebp, esp
	sub esp, 6*4

	cld
	mov edi, esp

	mov eax, cs
	stosd
	mov eax, ds
	stosd
	mov eax, ss
	stosd
	mov eax, es
	stosd
	mov eax, fs
	stosd
	mov eax, gs
	stosd

	mov eax, SEG_DATA16
	mov ds, eax
	mov ss, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	jmp SEG_CODE16:.16bit
.16bit:
BITS 16
	mov eax, cr0
	and al, 0xFE
	mov cr0, eax

	xor ax, ax
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	jmp 0x0000:.fix_cs
.fix_cs:
	sti

	mov si, word [bp + 16 + 4 + 4*2] ;//arg[2]
	mov dl, byte [bp + 16 + 4 + 4*1] ;//arg[1]
	mov cx, word [bp + 16 + 4 + 4*0] ;//arg[0]
	mov bx, .msg
	mov dh, byte [0x503]
	mov ax, word [0x506]
	mov es, ax
	mov di, word [0x504]
	xor ax, ax

	push ebp
	mov dword [.scratch_area], esp
	call dword 0x7C00
	mov esp, dword [.scratch_area]
	pop ebp

	cli

	mov ebx, cr0
	or bl, 1
	mov cr0, ebx

	mov ebx, eax

	lgdt [gdt_descriptor]

	cld
	mov esi, esp

	lodsd
	push ax
	push .32bit
	retf
.32bit:
BITS 32
	lodsd
	mov ds, eax
	lodsd
	mov ss, eax
	lodsd
	mov es, eax
	lodsd
	mov fs, eax
	lodsd
	mov gs, eax

	mov eax, ebx

	mov esp, ebp
	pop ebp
	pop edi
	pop esi
	pop ebx
	retd

.msg:
db "StOSrequ"
.scratch_area:
dd 0



_mbr_return:
	mov ax, SEG_DATA16
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov gs, ax
	mov fs, ax
	jmp SEG_CODE16:.16bits

BITS 16
.16bits:
	mov eax, cr0
	and ax, 0xFE
	mov cr0, eax

	xor ax, ax
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov sp, 0x7C00 ;//we are done anyways so don't care about the stack

	sti
	int 0x18

	jmp halt.jmp
BITS 32



;//cdecl
;//dword ds:si ptr
_mbr_transfer_control_flow:
	mov esi, [esp + 4]
	mov dl, byte [esi]
	mov edi, .scratch_area
	mov ecx, 4
	rep movsd

	test dl, dl
	jns .skip_active_replace

	;//We save disk number to MBR partinion table entry's active field
	;//because some old VBRs expect to find disk index there
	;//and modern ones usually does not complain about extra bits being set

	mov ebx, 0x7E00 - 2 - 64
	mov ecx, 4
.active_replace_loop:
	mov al, byte [ebx]
	test al, al
	cmovs eax, edx
	mov byte [ebx], al
	add ebx, 16
	loop .active_replace_loop
.skip_active_replace:

	mov ax, SEG_DATA16
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
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
	jmp 0x0000:.fix_cs
.fix_cs:
	mov si, .scratch_area
	mov bp, si
	mov di, [0x506]
	mov es, di
	mov di, [0x504]
	mov dh, [0x503]
	mov sp, 0x7C00
	sti

	xor eax, eax
	xor ebx, ebx
	xor ecx, ecx
	and edx, 0xFFFF
	and esi, 0xFFFF
	and edi, 0xFFFF
	and ebp, 0xFFFF

	jmp 0x7C00

.scratch_area:
times 16 db 0

BITS 32
