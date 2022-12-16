BITS 16

SECTION .boot

;Here's where the real deal begins
;The plan:
;	Flex off with cool loading message
;	Check for available memory
;	Perform CPU discovery
;	Enable A20 pin
;	Setup reasonable video mode with VBE
;	Go 32 bit protected mode
;	Perhaps transfer control over to C++?
;	Go 64 bit mode with identity page mapping
;	Setup -2GB address space mapping for the kernel
;	Flex off with some PCI commands
;	Load kernel and transfer the control

;Assumptions:
;	loaded at 0x600
;	large valid stack
;	CS = DS = ES = 0
;	IF=1

loader_entry:
	jmp short loader_start
	times 4 - ($ - loader_entry) nop

rodata:
	.str_logo db " StOS loader", 0

loader_start:
	mov ax, 0x0002
	int 0x10

	call endl
	mov si, rodata.str_logo
	call puts




halt:
	cli
.x:
	hlt
	jmp .x


;destroys ax, bx
endl:
	mov ax, 0x0E0D
	mov bx, 0x0007
	int 0x10
	mov al, 10
	int 0x10
	ret

;si = C string ptr
;destroys ax, bx
puts:
	mov ah, 0x0E
	mov bx, 0x0007
.loop:
	lodsb
	test al, al
	jz .ret
	int 0x10
	cmp al, 10
	jne .loop
	mov al, 13
	int 0x10
	jmp .loop
.ret:
	ret
