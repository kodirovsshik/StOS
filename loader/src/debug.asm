
global setup_exception_handlers

extern puts
extern put32x
extern halt



SECTION .rodata
rodata:
	.str_ud db 10, "#UD", 0
	.str_ss db 10, "#SS", 0
	.str_unknown_exception db 10, "UNKNOWN EXCEPTION", 0
	.str_excp_ip db " GENERATED, IP = 0x", 0



SECTION .text
BITS 16



ud_handler:
	mov si, rodata.str_ud
	jmp common_exception_handler

ss_handler:
	mov si, rodata.str_ss
	jmp common_exception_handler

unknown_exception_handler:
	mov si, rodata.str_unknown_exception
	;jmp common_exception_handler

;si = exception string
common_exception_handler:
	call puts ;exception name string
	
	mov si, rodata.str_excp_ip ;", IP = "
	call puts

	mov bp, sp
	movzx ax, [bp]
	call put32x
	
	mov word [bp], halt ;cry about it
	iret



setup_exception_handlers:
	mov eax, unknown_exception_handler
	mov cx, 7
	rep stosd
	mov dword [24], ud_handler
	mov dword [0xC*4], ss_handler
	ret