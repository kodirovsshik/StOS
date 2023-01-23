
global halt

extern kmain



SECTION .text
BITS 64

kernel_entry:
	jmp kmain

halt:
	hlt
	jmp halt