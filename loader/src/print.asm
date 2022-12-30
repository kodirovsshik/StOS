
global space
global endl
global putc
global puts
global put32u
global put32x
global putNx
global flush_stdout

global data.output_use_screen
global data.output_use_log
global data.output_use_serial


SECTION .data
data:
.output_log_ptr dw 0
.output_use_screen db 1
.output_use_log db 1
.output_use_serial db 1


SECTION .text
BITS 16



;destroys ax, bx
endl:
	mov al, 10
	jmp putc

;destroys ax, bx
space:
	mov al, ' '

;al = character
;destroys ax, bx
putc:
	call putc1
	
	cmp al, 10
	jne .ret

	mov al, 13
	call putc1
.ret:
	ret



;al = character
putc1:
	push ds
	push word 0
	pop ds
.n0:
	cmp byte [data.output_use_screen], 0
	je .n1
	call .put_screen
.n1:
	cmp byte [data.output_use_log], 0
	je .n2
	call .put_log
.n2:
	cmp byte [data.output_use_serial], 0
	je .n3
	call .put_serial
.n3:
	pop ds
	ret

.put_screen:
	mov ah, 0x0E
	mov bx, 0x0007
	int 0x10
	ret

.put_log:
	cmp al, 13
	je .put_log.ret
	push es
	push word 0x2000
	pop es
	mov bx, [data.output_log_ptr]
	mov [es:bx], al
	inc word [data.output_log_ptr]
	pop es

.put_log.ret:
	ret

.put_serial:
	push dx
	mov dx, [0x400]
	out dx, al
	pop dx
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
;destroys eax, bx, cx, dx
put32x:
	mov cx, 8

;eax = number
;cx = hex digits to print
;destroys eax, bx, cx, dx
putNx:
	jcxz .ret
	xor dx, dx
.l1:
	mov bx, ax
	and bx, 0xF
	mov bx, [bx + .digits]
	
	push bx
	inc dx

	shr eax, 4
	loop .l1
	jnz .l1

	mov cx, dx
.l2:
	pop ax
	call putc
	loop .l2
.ret:
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
