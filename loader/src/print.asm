
global space
global endl
global putc
global puts
global put32u
global put32x

SECTION .text
BITS 16



;destroys ax, bx
endl:
	mov ax, 0x0E0D
	mov bx, 0x0007
	int 0x10
	mov al, 10
	int 0x10
	ret



;destroys ax, bx
space:
	mov al, ' '

;al = character
;destroys ax, bx
putc:
	mov ah, 0x0E
	mov bx, 0x0007
	int 0x10
	cmp al, 10
	jne .ret
	mov al, 13
	int 0x10
.ret:
	ret



;ds:si = C string ptr
;destroys ax, bx, si
puts:
	lodsb
	test al, al
	jz .ret
	call putc
	jmp puts
.ret:
	ret



;eax=number
;destroys eax, bx, cx
put32x:

	mov cx, 8
.l1:
	mov bx, ax
	and bx, 0xF
	mov bx, [bx + .digits]
	push bx
	shr eax, 4
	loop .l1

	mov cx, 8
.l2:
	pop ax
	call putc
	loop .l2

	ret

.digits:
	db '0123456789ABCDEF'

;eax = number
;destroys eax, ebx, cx, edx
put32u:
	xor cx, cx
	mov ebx, 10

.l1:
	xor edx, edx
	div ebx
	push dx
	inc cx
	test eax, eax
	jnz .l1

.l2:
	pop ax
	add al, '0'
	call putc
	loop .l2

	ret
