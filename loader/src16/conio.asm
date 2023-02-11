
global space
global endl
global putc
global puts
global put32u
global put32x
global putNx
global flush_stdout
global cputc
global cputs
global cput32u
global cput32x
global __fill_conio_params
global save_output_buffer

global data.output_use_screen
global data.output_use_log
global data.output_use_serial

extern pbr_disk
extern edata.output_buffer_index


SECTION .data
data:
.output_use_screen db 1
.output_use_log db 1
.output_use_serial db 1


SECTION .text
BITS 16



;cdecl args:
;	void* pre to conio params table
__fill_conio_params:
	mov eax, [esp + 4]
	mov dword [eax + 0], data.output_use_screen
	mov dword [eax + 4], data.output_use_log
	mov dword [eax + 8], data.output_use_serial
	o32 ret



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



;cdecl args:
;	char character
cputc:
	push bx
	mov al, [ss:esp + 6]
	call putc
	pop bx
	o32 ret



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
	mov bx, [edata.output_buffer_index]
	push ds
	push word 0x1000
	pop ds
	mov [bx], al
	pop ds
	inc bx
	mov [edata.output_buffer_index], bx

.put_log.ret:
	ret

.put_serial:
	push ds
	push word 0
	pop ds

	push dx
	mov dx, [0x400]
	out dx, al
	pop dx

	pop ds
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



;cdecl args:
;	bool break_line
;	farptr string
cputs:
	push ebx
	push esi

	mov esi, [ss:esp + 12]
	mov eax, esi
	shr eax, 16
	push ds
	push ax
	pop ds
	call puts
	pop ds

	mov ax, [ss:esp + 16]
	test al, al
	jz .done
	call endl

.done:
	pop esi
	pop ebx
	o32 ret



;cdecl args:
;	uint16_t minimum digits
;	uint32_t value
cput32x:
	push ebp
	mov bp, sp
	push ebx

	xor cx, cx
	mov eax, [bp + 8]
.divide_loop:
	mov edx, eax
	and edx, 0xF
	add dx, .digits
	push word [edx]

	inc cx

	shr eax, 4
	cmp cx, [bp + 12]
	jb .divide_loop
	test eax, eax
	jnz .divide_loop

.print_loop:
	pop ax
	call putc
	loop .print_loop

	pop ebx
	pop ebp
	o32 ret
.digits:
	db "0123456789ABCDEF"



;cdecl args:
;	uint16_t minimum digits
;	uint32_t value
cput32u:
	push ebp
	mov bp, sp
	push ebx
	push dword 10

	xor cx, cx
	mov eax, [bp + 8]
.divide_loop:
	xor edx, edx
	div dword [bp - 8]
	add dx, .digits
	push word [edx]

	inc cx

	cmp cx, [bp + 12]
	jb .divide_loop
	test eax, eax
	jnz .divide_loop

.print_loop:
	pop ax
	call putc
	loop .print_loop

	lea sp, [bp - 4]
	pop ebx
	pop ebp
	o32 ret
.digits:
	db "0123456789"



;eax=number
;destroys eax, cx, dx
put32x:
	mov cx, 8

;eax = number
;cx = hex digits to print
;destroys eax, cx, dx
putNx:
	push ecx
	push eax
	call dword cput32x
	add esp, 8
	ret



;eax = number
;destroys eax, cx, edx
put32u:
	push dword 0
	push eax
	call dword cput32u
	add esp, 8
	ret



save_output_buffer:
	;64 KiB at 0x10000 are written to 128 sectors right before 1 MiB boundary
	mov bx, [edata.output_buffer_index]

	;Write terminating zero at the end of the buffer
	push ds
	push 0x1000
	pop ds
	mov byte [bx], 0
	pop ds

	mov dl, [pbr_disk]

	;IO of 64KiB at once is not supported by some BIOSes' int13 extensions
	;(Including my development machine with most recent BIOS, for some reason??)
	;workaround: split write task into 2 chunks of 32KiB

	push dword 0
	push dword 0x00000780 
	push dword 0x10000000
	push dword 0x00400010
	mov si, sp

	mov ax, 0x4300
	int 0x13

	add word [esp + 8], 0x40 ;Advance by # of sectors written before
	add word [esp + 4], 0x8000 ;Advance to the second half of the buffer

	mov ax, 0x4300
	int 0x13

	add sp, 16
	ret
