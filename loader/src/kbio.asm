
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



wait_enter_hit:
	push ax
.l:
	call getch
	cmp al, 13
	jne .l
	pop ax
	ret
