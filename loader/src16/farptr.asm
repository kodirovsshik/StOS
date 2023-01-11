
global far_read
global far_write



SECTION .text
BITS 16


;cdecl args:
;	uint16_t off
;	uint16_t seg
far_read:
	push es
	push word [esp + 6] ;segment
	pop es

	xor eax, eax
	mov ax, [esp + 10] ;offset

	mov eax, [es:eax]

	pop es
	o32 ret



;cdecl args:
;	uint32_t value to write
;	uint16_t off
;	uint16_t seg
far_write:
	push es
	push word [esp + 6] ;segment
	pop es

	xor eax, eax
	mov ax, [esp + 10] ;offset

	mov edx, [esp + 14]
	mov [es:eax], edx

	pop es
	o32 ret
