BITS 16

SECTION .boot
ORG 0x600

;Here's where the real deal begins
;The plan:
;	Flex off with cool loading message
;	Perform CPU discovery
;	Check for available memory
;	Enable A20 pin
;	Setup reasonable video mode with VBE
;	Setup protected mode environment 
;	Go 32 bit mode (Perhaps transfer the control over to C++?)
;	Setup 64-bit environment
;	Go 64 bit mode with identity page mapping
;	Setup -2GB address space mapping for the kernel
;	Flex off with some PCI commands
;	Load kernel and transfer the control

;Assumptions:
;	loaded at 0x600
;	large valid stack
;	CS = DS = ES = 0
;	IF=1

%define __loader_img_size ((loader_end - loader_begin + 3) / 4 * 4)

loader_begin:
	jmp short loader_main
	times 4 - ($ - loader_begin) nop

rodata:
	.str_logo db 10, "StOS loader 1.0", 10, 0
	.str_cpu db "CPU vendor: ", 0
	.str_e820_err db "Warning: E820 in unavailable", 10, 0

data:
	.heap dw 0x600 + __loader_img_size
	.memory_map_addr dw 0
	.memory_map_size dw 0

loader_main:
	mov ax, 0x0002
	int 0x10

	mov si, rodata.str_logo
	call puts

	cli
;CPU discovery with Intel's algorithm
;Assume at least 8086
	pushf
	pop ax
	mov cx, ax ;save old flags for later
	and ax, 0x0FFF ;on 8086/80186 bits 12-15 of FLAGS are always 1
	push ax
	popf
	pushf
	pop ax
	and ax, 0xF000
	cmp ax, 0xF000
	je err_unsupported_cpu ;8086 detected

;Assume at least 80286
	or cx, 0xF000
	push cx
	popf
	pushf
	pop ax
	and ax, 0xF000 
	;I don't understand this one, why would all of them be 0 only on 80286?
	jz err_unsupported_cpu ;80286 detected

;Assume at least 80386 - 32 bit registers are present
	pushfd
	pop eax
	mov ecx, eax
	xor eax, 0x40000 ;you can't switch this flag until 80486
	push eax
	popfd
	pushfd
	pop eax
	cmp eax, ecx
	je err_unsupported_cpu ;80386 detected
	
	;Restore AC bit
	push ecx
	popfd

;Assume at least 80486
	mov eax, ecx
	xor eax, 0x200000 ;CPUID flag is flippable iff CPUID is available
	push eax
	popfd
	pushfd
	pop eax
	cmp eax, ecx
	je err_unsupported_cpu ;couldn't flip CPUID flag, early 80486 detected

	mov eax, 0x80000000
	cpuid
	mov ebx, 0x80000001 
	cmp eax, ebx ;need extended page 1 for long mode
	jb err_unsupported_cpu

	mov eax, ebx
	cpuid
	test edx, 1 << 29 ;long mode bit
	jz err_unsupported_cpu

;Supported CPU detected, CPU discovery done

	shl esp, 16
	shr esp, 16

	push dword 0x200
	popfd ;IF=1

	mov si, rodata.str_cpu
	call puts

	sub esp, 16
	mov edi, esp
	xor eax, eax
	cpuid
	mov eax, ebx
	stosd
	mov eax, edx
	stosd
	mov eax, ecx
	stosd
	mov eax, 10
	stosd

	mov si, sp
	call puts

	add sp, 16

	call try_fill_memory_table_e820
	jnc memory_table_done

	mov si, rodata.str_e820_err
	call puts
	
memory_table_done:

	mov si, [data.memory_map_addr]
	xor ecx, ecx
	mov cx, [data.memory_map_size]
	
.l1:
	push cx

	mov eax, [si + 4]
	call put32X
	mov eax, [si]
	call put32X

	mov al, ' '
	call putc

	mov eax, [si + 12]
	call put32X
	mov eax, [si + 8]
	call put32X

	mov al, ' '
	call putc

	mov eax, [si + 16]
	call put32X

	mov al, ' '
	call putc

	mov eax, [si + 20]
	call put32X

	call endl

	add si, 24
	pop cx

	test cx, 7
	jnz .doloop
	xor ah, ah
	int 0x16

.doloop:
	loop .l1

	xor eax, eax
	mov ax, [data.memory_map_size]
	call put32X
	call endl

	jmp halt








try_fill_memory_table_e820:
	pushad
	cld

	call heap_get_ptr
	mov si, ax

	mov eax, 0x0000E820
	mov edx, 0x534D4150
	xor ebx, ebx
	mov ecx, 24
	mov edi, 0x504
	int 0x15
	mov ecx, eax
	lahf
	cmp ecx, 0x534D4150
	jne .ret0

	mov [data.memory_map_addr], si

.loop:
	xchg si, di
	mov cx, 6
	rep movsd

	mov si, di
	mov di, 0x504
	
	inc word [data.memory_map_size]
	
	sahf
	jc .ret1
	test ebx, ebx
	jz .ret1
	
	mov eax, 0x0000E820
	mov ecx, 24
	mov edx, 0x534D4150
	int 0x15
	lahf
	jmp .loop

.ret1:
	call heap_set_ptr
	popad
	clc
	ret
.ret0:
	popad
	stc
	ret

err_unsupported_cpu:
	push word 0x200
	popf
	mov si, .str
	call puts
	jmp halt
.str:
	db "Unsupported CPU detected, 64 bit support required", 0

halt:
	sti
	mov si, .str
	call puts
.x:
	hlt
	jmp .x
.str:
	db 10, "CPU HALTED", 10, 0

;return AX = heap ptr
heap_get_ptr:
	mov ax, [data.heap]
	ret

;si = ptr
heap_set_ptr:
	mov [data.heap], si
	ret

;stack (callee-poped)
; uint16_t allocation size
;return AX = memory ptr
linear_alloc:
	push bx
	mov bx, sp
	mov bx, [bx + 4]
	mov ax, [data.heap]
	add [data.heap], bx
	pop bx
	ret 2


;destroys ax, bx
endl:
	mov ax, 0x0E0D
	mov bx, 0x0007
	int 0x10
	mov al, 10
	int 0x10
	ret

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

;si = C string ptr
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
;destroys ax, bx, cx
put32X:

	mov cx, 8
.l1:
	mov bx, ax
	and bx, 0xF
	add bx, .digits
	mov bx, [bx]
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

loader_end:
