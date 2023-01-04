
global ind
global outd

SECTION .text
BITS 16



;cdecl args:
;	uint16_t port
ind:
	mov dx, [esp + 4]
	in eax, dx
	o32 ret



;cdecl args:
;	uint32_t data
;	uint16_t port
outd:
	mov dx, [esp + 4]
	mov eax, [esp + 8]
	out dx, eax
	o32 ret
