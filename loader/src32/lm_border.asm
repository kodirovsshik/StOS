
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
.tss:
	dd 0x00000068
	dd 0x00C08900
	dq 0
.end:

%define seg64_code (gdt64.code - gdt64)
%define seg64_data (gdt64.data - gdt64)



idt_handlers:
	times 13 dd 0
	dd gp_handler
	times 18 dd 0



SECTION .data



idtr64:
	dw idt64.end - idt64
	dd idt64
	dd 0

idt64:
	times 32*16 db 0
.end:



SECTION .text



gp_handler:
BITS 64
	mov rax, 0x0123456789ABCDEF
	iret
	hlt
	jmp gp_handler


BITS 32
go_lm:
	mov eax, 0x30000
	mov cr3, eax

	mov eax, cr4
	or eax, 1 << 5 ;PAE
	mov cr4, eax

	mov ecx, 0xC0000080 ;EFER
	rdmsr
	or eax, 1 << 8 ;LME
	wrmsr

	mov eax, cr0
	or eax, 1 << 31 ;PG
	mov cr0, eax

	nop

	lidt [idtr64]
	lgdt [gdtr64]

	mov eax, gdt64.tss - gdt64
	ltr ax

	mov eax, seg64_data
	mov ds, eax
	mov ss, eax
	mov es, eax

	jmp seg64_code:lm_bootstrap



BITS 64
lm_bootstrap:
	mov rax, 0xFFFFFFFF80000000
	mov edi, edata
	jmp rax
