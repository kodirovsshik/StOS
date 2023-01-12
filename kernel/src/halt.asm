
global halt



SECTION .text
BITS 64



halt:
	hlt
	jmp halt
