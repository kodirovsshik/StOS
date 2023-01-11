
global outb
global inb



SECTION .text
BITS 32



;cdecl args:
;	uint16_t port
;return:
;	eax = uint8_t value
inb:
	mov edx, [esp + 4]
	xor eax, eax
	in al, dx
	ret



;cdecl args:
;	uint8_t value
;	uint16_t port
;return:
;	eax = uint8_t value
outb:
	mov edx, [esp + 4]
	mov al, [esp + 8]
	out dx, al
	ret
