
extern panic
extern puts
extern sleep

global activate_a20



SECTION .text
BITS 16


activate_a20:
	;first check if already done
	call check_a20_fast
	jc .a20_done

	;then try BIOS method
	mov ax, 0x2401
	int 0x15
	call check_a20_fast
	jc .a20_done

	;then try keyboard controller method
	call try_enable_a20_keyboard_controller_method
	call check_a20_slow
	jc .a20_done

	;then try 0xEE port method
	in al, 0xEE
	call check_a20_slow
	jc .a20_done

	;then try 0x92 port method
	call try_enable_a20_port92
	call check_a20_slow
	jc .a20_done

	;if none worked, give up
	jmp a20_fail
.a20_done:
	ret



try_enable_a20_keyboard_controller_method:
	ret



;destroys al
try_enable_a20_port92:
	in al, 0x92
	;Best practice to avoid using this method
	;if A20 bit is already set
	test al, 2
	jnz .ret

	or al, 2
	and al, ~1 ;bit 0 is fast reset
	out 0x92, al
.ret:
	ret



;return:
;CF=1 if A20 enabled
;CF=0 otherwise
check_a20_fast:
	mov cx, 5
	jmp check_a20



;return:
;CF=1 if A20 enabled
;CF=0 otherwise
check_a20_slow:
	mov cx, 100
	;jmp check_a20

;CX = checks count > 0
;return:
;CF=1 if A20 enabled
;CF=0 otherwise
check_a20:
.l1:
	mov ax, 2
	push cx
	call sleep
	call _check_a20
	pop cx
	jc .ok
	loop .l1

	clc
	ret
.ok:
	stc
	ret



;return:
;CF=1 if A20 enabled
;CF=0 otherwise
_check_a20:
	mov ax, 0xFF00
	mov es, ax

	wbinvd
	mov dword es:[0x1500], 0x55AA0000
	wbinvd
	mov dword ds:[0x0500], 0xAA550000

	wbinvd
	mov eax, es:[0x1500]
	wbinvd
	sub eax, ds:[0x0500]

	mov es, ax
	ret



;noreturn
a20_fail:
	mov si, .str
	jmp panic
.str:
	db "Failed to activate A20 line", 0
