
global go_lm
global idt_handlers
global idt64

extern edata


SECTION .rodata



gdtr64:
	dw gdt64.end - gdt64 - 1
	dd gdt64
	dd 0

gdt64:
	dq 0
.code:
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 0x98
	db 0xAF
	db 0x00
.data:
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 0x92
	db 0xCF
	db 0x00
	dd 0x00000068
	dd 0x00C08900
	dq 0
.end:

%define seg64_code (gdt64.code - gdt64)
%define seg64_data (gdt64.data - gdt64)




SECTION .text



BITS 32
go_lm:
	mov eax, 0x30000
	mov cr3, eax

	mov eax, cr4
	or eax, 1 << 5 ;PAE
	mov cr4, eax

	mov ecx, 0xC0000080 ;EFER
	rdmsr
	or eax, (1 << 8) | (1 << 11) ;LME, NXE
	wrmsr

	mov eax, cr0
	or eax, 1 << 31 ;PG
	mov cr0, eax

	nop

	lgdt [gdtr64]

	mov eax, seg64_data
	mov ds, eax
	mov ss, eax
	mov es, eax

	jmp seg64_code:lm_bootstrap



BITS 64
lm_bootstrap:
	;System V x86-64 abi
	;First argument (for kernel) passed in rdi
	mov edi, edata

	;Stack must be 16-byte aligned upon a function call
	mov esp, 0x10000
	
	;Why is there no better way to do an absolute jump
	mov rax, 0xFFFFFFFF80000000
	jmp rax
