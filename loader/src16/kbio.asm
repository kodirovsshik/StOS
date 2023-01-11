
global wait_enter_hit
global getch

SECTION .text
BITS 16



;ah = scan code
;al = ASCII symbol
getch:
	xor ah, ah
	int 0x16
	ret



;registers preserved
wait_enter_hit:
	push ax
.wait_key_press:
	call getch
	cmp al, 13
	jne .wait_key_press
	pop ax
	ret
