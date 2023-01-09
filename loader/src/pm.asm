
global go_pm



SECTION .rodata

GDTR:
	dw _gdt.end - _gdt - 1
	dd _gdt

_gdt:
	dq 0

.code32:
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 0x98
	db 0xCF
	db 0x00

.data32:
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 0x92
	db 0xCF
	db 0x00

.code16:
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 0x98
	db 0x80
	db 0x00

.data16:
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 0x92
	db 0x80
	db 0x00

.end:

%define seg_code32 (_gdt.code32 - _gdt)
%define seg_data32 (_gdt.data32 - _gdt)
%define seg_code16 (_gdt.code16 - _gdt)
%define seg_data16 (_gdt.data16 - _gdt)


str_pm: db 10, "PROTECTED MODE", 10
.end:


SECTION .text

BITS 16
go_pm:
	cli

	mov al, 0x80
	out 0x70, al
	in al, 0x71

	lgdt [GDTR]
	mov eax, cr0
	or ax, 1
	mov cr0, eax

	mov ax, seg_data32
	mov ds, ax
	mov ss, ax
	jmp dword seg_code32:.in_pm

BITS 32
.in_pm:
	mov eax, 0x12345678
	xor ax, ax

	mov esi, str_pm
	mov ecx, str_pm.end - str_pm
	mov dx, [0x400]
	rep outsb

	mov ax, seg_data16
	mov ds, ax
	mov ss, ax
	jmp seg_code16:.leave
BITS 16
.leave:

	mov eax, cr0
	and ax, ~1
	mov cr0, eax

	xor eax, eax
	mov ds, eax
	mov ss, eax
	jmp 0:.ret
.ret:
	o32 ret
