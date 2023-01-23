
global go_pm
global interrupt

extern pm_main
extern go_lm



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
	db 0x00
	db 0x00

.data16:
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 0x92
	db 0x00
	db 0x00

.end:

%define seg_code32 (_gdt.code32 - _gdt)
%define seg_data32 (_gdt.data32 - _gdt)
%define seg_code16 (_gdt.code16 - _gdt)
%define seg_data16 (_gdt.data16 - _gdt)



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
	call pm_main
	test eax, eax
	jns go_lm

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
	jmp 0:.cleanup

.cleanup:
	mov al, 0x00
	out 0x70, al
	in al, 0x71

	sti

.ret:
	o32 ret



;cdecl args:
;	uint8_t vector
;	void* regs_ptr
BITS 32
interrupt32:
interrupt:
	push ebp
	mov ebp, esp
	;sub esp, 0
	pushad

	mov al, [ebp + 12]
	mov [.vector], al

	mov ebp, [ebp + 8]
	push ebp

.pm16_enter:
	mov eax, seg_data16
	mov ds, eax
	mov ss, eax
	jmp seg_code16:.pm16

BITS 16
.pm16:
	mov eax, cr0
	and al, ~1
	mov cr0, eax

	xor ax, ax
	mov ds, ax
	mov ss, ax
	jmp 0:.rm

.rm:
	mov eax, [bp + 0]
	mov ecx, [bp + 4]
	mov edx, [bp + 8]
	mov ebx, [bp + 12]
	mov esi, [bp + 16]
	mov edi, [bp + 20]
	mov ebp, [bp + 24]

	sti
	db 0xCD
.vector:
	db 0
	cli

	pushfd
	push ebp

	mov ebp, [esp + 8]
	pop dword [bp + 28]
	pop dword [bp + 24]
	add sp, 4

	mov [bp + 20], edi
	mov [bp + 16], esi
	mov [bp + 12], ebx
	mov [bp + 8], edx
	mov [bp + 4], ecx
	mov [bp + 0], eax

.pm32_enter:
	mov eax, cr0
	or al, 1
	mov cr0, eax

	mov ax, seg_data32
	mov ss, ax
	mov ds, ax
	jmp seg_code32:.pm32

BITS 32
.pm32:
	nop
.ret:
	popad
	mov esp, ebp
	pop ebp
	ret
