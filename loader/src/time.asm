
global delay
global sleep

SECTION .text
BITS 16

;destroys ax
delay:
	mov ax, 2
	;jmp sleep

;ax = time in milliseconds
sleep:
	pushad
	mov bx, 1000
	mul bx
	mov cx, dx
	mov dx, ax
	mov ah, 0x86
	int 0x15
	popad
	ret
