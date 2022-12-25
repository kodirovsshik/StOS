
SECTION .text
BITS 16

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
;	loaded at 0x0000:0x0600

%define __loader_img_size ((loader_end - loader_begin + 3) / 4 * 4)

loader_begin:
	jmp loader_main


rodata:
	.str_logo db 10, "StOS loader v1.0", 10, 0
	.str_mem1 db " KiB (", 0
	.str_mem2 db " MiB) usable memory", 10, 0
	.str_vbe db "VBE ", 0
	.str_vbe_modes1 db " modes reported, ", 0
	.str_vbe_modes2 db " modes usable", 10, 0



align 4, db 0
data:
	.heap dw 0x600 + __loader_img_size
	.has_memory_over_1m db 0
	db 0

align 4, db 0
edata: ;data to be exported later for kernel
	.memory_map_addr dw 0
	.memory_map_size dw 0
	.memory_at_1m dw 0
	.memory_at_16m dw 0
	.e820_ok db 1
	db 0
	.io_ports times 7 dw 0
	.vbe_modes_ptr dw 0
	.vbe_modes_count dw 0
	.vbe_modes_count_reported dw 0



loader_main:
	cli

	xor ax, ax
	mov ds, ax
	mov es, ax

	mov sp, ax
	mov ax, 0x7000
	mov ss, ax

	sti

	mov ax, 0x0002
	int 0x10

	mov si, rodata.str_logo
	call puts

	cli


.cpu_discovery: ;Intel's algorithm

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

;Assume at least 80386 => 32 bit registers are present
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
	je err_unsupported_cpu ;couldn't flip CPUID flag

;CPUID present
	mov eax, 0x80000000
	cpuid
	mov ebx, 0x80000001
	cmp eax, ebx ;need extended page 1 for long mode
	jb err_unsupported_cpu

	mov eax, ebx
	cpuid
	test edx, 1 << 29 ;long mode bit
	jz err_unsupported_cpu

.cpu_done:
;Supported CPU detected, CPU discovery done

	xor esp, esp

	push dword 0x200
	popfd ;IF=1

	call print_cpu_data

	call try_fill_memory_table_e820
	jnc .memory_table_ok
	;else use fallback methods
	mov byte [edata.e820_ok], 0

	call get_e801_info
	call try_fill_memory_table_int12
	jnc .memory_table_ok

	jmp err_memory_detection

.memory_table_ok:

	call get_memory_size_KiB
	push eax

	call put32u
	mov si, rodata.str_mem1
	call puts

	pop eax
	add eax, 513
	shr eax, 10
	call put32u
	mov si, rodata.str_mem2
	call puts

	;call print_memory_map


	cmp byte [data.has_memory_over_1m], 1
	jne .a20_done


.a20:
	nop
	call check_a20_fast
	jc .a20_done

	mov ax, 0x2401
	int 0x15
	call check_a20_fast
	jc .a20_done

	call try_enable_a20_kb
	call check_a20_slow
	jc .a20_done

	in al, 0xEE
	call check_a20_slow
	jc .a20_done

	call try_enable_a20_port92
	call check_a20_slow
	jc .a20_done

	jmp a20_fail

.a20_done:
	mov si, 0x400
	mov di, edata.io_ports
	mov cx, 7
	rep movsw

.vbe:
	mov bx, ss
	mov es, bx

	sub sp, 512
	mov di, sp
	mov dword [ss:di], 0x32454256 ;'VBE2'
	mov ax, 0x4F00
	int 0x10

	xor bx, bx
	mov es, bx

	cmp ax, 0x4F
	jne vbe_err
	inc byte [vbe_err.code]

.vbe_print_rev:
	mov si, rodata.str_vbe
	call puts

	xor eax, eax
	mov al, [ss:di + 5]
	call put32u

	mov al, '.'
	call putc

	xor eax, eax
	mov al, [ss:di + 4]
	call put32u
	call endl

	cmp byte [ss:di + 5], 3
	jb vbe_err_old

.vbe_print_modes:
	mov eax, [ss:di + 14]
	call vbe_copy_video_modes

	xor eax, eax
	mov ax, [edata.vbe_modes_count_reported]
	call put32u
	mov si, rodata.str_vbe_modes1
	call puts

	xor eax, eax
	mov ax, [edata.vbe_modes_count]
	call put32u
	mov si, rodata.str_vbe_modes2
	call puts

	push di
	;call vbe_print_video_modes
	pop di

.vbe_print_oem:
	mov si, [ss:di + 6]
	mov ax, [ss:di + 8]
	mov ds, ax
	call puts
	xor ax, ax
	mov ds, ax
	call endl

.vbe2_test:
	mov al, [ss:di + 5]
	cmp al, 2
	jb .vbe_done

.vbe_print_vendor_name:
	mov si, [ss:di + 22]
	mov ax, [ss:di + 24]
	mov ds, ax
	call puts
	xor ax, ax
	mov ds, ax
	call endl
.vbe_print_product_name:
	mov si, [ss:di + 26]
	mov ax, [ss:di + 28]
	mov ds, ax
	call puts
	xor ax, ax
	mov ds, ax

	mov al, ' '
	call putc
.vbe_print_product_rev:
	mov si, [ss:di + 30]
	mov ax, [ss:di + 32]
	mov ds, ax
	call puts
	xor ax, ax
	mov ds, ax
	call endl

.vbe_done:
	add sp, 512
	nop


.prehalt:
	jmp halt



vbe_print_video_modes:
	mov si, [edata.vbe_modes_ptr]
	mov cx, [edata.vbe_modes_count]
	test cx, cx
.l:
	jz .ret
	lodsw
	call wait_enter_hit
	push si
	push cx
	call vbe_print_video_mode
	pop cx
	pop si
	dec cx
	jmp .l

.ret:
	ret



;ax = video mode number
vbe_print_video_mode:
	push es
	
	sub sp, 256
	mov di, sp
	push ss
	pop es

	mov cx, ax
	mov ax, 0x4f01
	int 0x10

	mov bx, [es:di]
	and bx, 1
	mov al, [bx + .su]
	call putc

	mov bx, [es:di]
	shr bx, 2
	and bx, 1
	mov al, [bx + .tx]
	call putc

	mov bx, [es:di]
	shr bx, 3
	and bx, 1
	mov al, [bx + .mc]
	call putc

	mov bx, [es:di]
	shr bx, 4
	and bx, 1
	mov al, [bx + .tg]
	call putc

	mov bx, [es:di]
	shr bx, 5
	and bx, 1
	mov al, [bx + .vx]
	call putc

	mov bx, [es:di]
	shr bx, 7
	and bx, 1
	mov al, [bx + .lw]
	call putc

	mov al, ' '
	call putc

	xor eax, eax
	mov ax, [es:di + 18]
	call put32u

	mov al, 'x'
	call putc

	xor eax, eax
	mov ax, [es:di + 20]
	call put32u

	mov al, 'x'
	call putc

	xor eax, eax
	mov al, [es:di + 25]
	call put32u

	mov al, ' '
	call putc

	xor eax, eax
	mov al, [es:di + 27]
	call put32u

	call endl

	add sp, 256
	pop es
	ret
.su db "US" ;supported
.tx db "tT" ;bios tty supported
.mc db "MC" ;monochrome/color
.tg db "TG" ;text/graphics
.vx db "Vv" ;vga compatible
.lw db "lL" ;linear frame buffer



;eax = VbeFarPtr to modes array
;modifies edata
vbe_copy_video_modes:
	pushad
	sub sp, 256

	push ax
	
	call heap_get_ptr
	mov [edata.vbe_modes_ptr], ax
	mov di, ax

	xor ax, ax
	mov es, ax
	
	pop ax

	mov si, ax
	shr eax, 16
	mov ds, ax

.l:
	lodsw
	cmp ax, 0xFFFF
	je .le
	inc word [es:edata.vbe_modes_count_reported]
	call vbe_check_mode
	jc .l
	stosw
	inc word [es:edata.vbe_modes_count]
	jmp .l
.le:

	xor ax, ax
	mov ds, ax

	call heap_set_ptr

	add sp, 256
	popad
	ret



;ax = mode
;CF=1 if bad
;CF=0 if good
vbe_check_mode:
	pushad
	push es
	
	push ss
	pop es
	sub sp, 256
	mov di, sp

	mov cx, ax
	mov ax, 0x4F01
	int 0x10
	mov ax, [es:di]
	test ax, 1
	jz .bad

.good:
	clc
	jmp .ret
.bad:
	stc
.ret:
	lea sp, [esp + 256]
	pop es
	popad
	ret



vbe_err:
	mov si, .str
	call puts

	xor eax, eax
	mov al, [.code]

	push eax
	call put32u
	call endl

	jmp halt
.code:
	db 0
.str:
	db "VBE error, counter: "



vbe_err_old:
	mov si, .str
	call puts
	jmp halt
.str:
	db "VBE 3.0 or newer required", 10, 0



try_enable_a20_kb:
	ret



try_enable_a20_port92:
	in al, 0x92
	test al, 2
	jnz .ret
	or al, 2
	and al, ~1
	out 0x92, al
.ret:
	ret



a20_fail:
	mov si, .str
	call puts
	jmp halt
.str:
	db "Failed to activate A20 line", 10, 0



;return:
;eax = memory size in KiB
;destroys eax, bx, cx, edx
get_memory_size_KiB:
	xor eax, eax
	mov bx, [edata.memory_map_addr]
	mov cx, [edata.memory_map_size]

.l1:
	cmp dword [bx + 16], 1
	jne .next
	test dword [bx + 20], 1
	jz .next
	mov edx, [bx + 8]
	add edx, 512
	pushf
	shr edx, 10
	add eax, edx
	mov edx, [bx + 12]
	popf
	adc edx, 0
	shl edx, 22
	add eax, edx
.next:
	add bx, 24
	loop .l1

.include_high: 
	xor edx, edx
	mov dx, [edata.memory_at_1m]
	add eax, edx
	mov dx, [edata.memory_at_16m]
	shl edx, 6
	add eax, edx
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



;destroys ax
delay:
	mov ax, 2
	;jmp sleep

;ax = time in milliseconds
sleep:
	pusha
	mov bx, 1000
	mul bx
	mov cx, dx
	mov dx, ax
	mov ah, 0x86
	int 0x15
	popa
	ret



;return:
;CF=1 if A20 enabled
;CF=0 otherwise
_check_a20:
	mov ax, 0xFF00
	mov fs, ax

	wbinvd
	mov dword ds:[0x0504], 0xAA550000
	wbinvd
	mov dword fs:[0x1504], 0x55AA0000

	wbinvd
	mov eax, fs:[0x1504]
	wbinvd
	sub eax, ds:[0x0504]

	mov fs, ax
	ret



;fills edata section
get_e801_info:
	xor cx, cx
	xor dx, dx

	mov ax, 0xE801
	int 0x15
	jc .ret

	jcxz .use_ab
	mov ax, cx
	mov bx, dx
.use_ab:
	mov [edata.memory_at_1m], ax
	mov [edata.memory_at_16m], bx

	test ax, ax
	setnz dl
	or byte [data.has_memory_over_1m], dl

	test bx, bx
	setnz dl
	or byte [data.has_memory_over_1m], dl

	clc
.ret:
	ret



try_fill_memory_table_int12:
	xor eax, eax
	int 0x12
	jc .fallback

.ok:
	shl eax, 10
	push eax
	call set_low_memory_size_KiB
	
	clc
	ret

.fallback:
	xor eax, eax
	mov ax, [0x413]

	cmp ax, 32
	jb .fail
	cmp ax, 640
	jbe .ok
	
.fail:
	stc
	ret



;stack (callee-popped):
; uint32_t memory size (in bytes)
set_low_memory_size_KiB:
	mov word [edata.memory_map_size], 1

	push word 24
	call linear_alloc
	mov word [edata.memory_map_addr], ax
	mov di, ax

	xor eax, eax
	times 2 stosd
	mov eax, dword [esp + 2]
	stosd
	xor eax, eax
	stosd
	inc ax
	times 2 stosd
	ret 4



print_memory_map:
	mov si, [edata.memory_map_addr]
	mov cx, [edata.memory_map_size]
	
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

	test cx, 3
	jnz .doloop

	call wait_enter_hit

.doloop:
	loop .l1

	xor eax, eax
	mov ax, [edata.memory_map_size]
	call put32X
	call endl

	ret



wait_enter_hit:
	push ax
.l:
	call getch
	cmp al, 13
	jne .l
	pop ax
	ret


	
getch:
	xor ah, ah
	int 0x16
	ret



print_cpu_data:
	push es
	sub sp, 52

	mov ax, ss
	mov ds, ax
	mov es, ax
	mov di, sp
	mov si, sp
	
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
	call puts

	mov eax, 0x80000000
	cpuid
	cmp eax, 0x80000004
	jb .ret


	mov di, sp
	mov si, sp

	mov eax, 0x80000002
	cpuid
	stosd
	mov eax, ebx
	stosd
	mov eax, ecx
	stosd
	mov eax, edx
	stosd

	mov eax, 0x80000003
	cpuid
	stosd
	mov eax, ebx
	stosd
	mov eax, ecx
	stosd
	mov eax, edx
	stosd

	mov eax, 0x80000004
	cpuid
	stosd
	mov eax, ebx
	stosd
	mov eax, ecx
	stosd
	mov eax, edx
	stosd
	
	xor eax, eax
	stosd

.unpad: ;why does intel pad their CPU strings with spaces
	mov al, [ds:si]
	cmp al, ' '
	jne .print
	inc si
	jmp .unpad

.print:
	call puts
	call endl

.ret:
	xor ax, ax
	mov ds, ax
	add sp, 52
	pop es
	ret



try_fill_memory_table_e820:
	pushad
	cld

	;EBX = memory map index
	;DI = current heap ptr (progressively advances after each int 0x15 call)
	;AX, CX, DX used as temporaries or as arguments for interrupts

	call heap_get_ptr
	mov di, ax

	xor ebx, ebx

	mov eax, 0x0000E820
	mov ecx, 24
	mov edx, 0x534D4150
	int 0x15
	
	jc .ret0
	cmp eax, 0x534D4150
	jne .ret0

	mov [edata.memory_map_addr], di

.loop:
	test ebx, ebx
	jz .ret1

	inc word [edata.memory_map_size]

	cmp cx, 20
	ja .loop1
	
.no_acpi3:
	mov dword [di + 20], 1
.loop1:
	
	mov ecx, [di + 0]
	mov edx, [di + 4]
	add ecx, [di + 8]
	adc edx, [di + 12]

	test edx, edx
	jnz .set_1m_flag
	cmp ecx, 0x100000
	jbe .skip_set_1m_flag

.set_1m_flag:
	mov byte [data.has_memory_over_1m], 1

.skip_set_1m_flag:
	mov ecx, 24
	add di, cx
	mov eax, 0x0000E820
	mov edx, 0x534D4150
	int 0x15
	jnc .loop

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
	db "Unsupported CPU detected, 64 bit support required", 10, 0



err_memory_detection:
	mov si, .str
	call puts
	jmp halt
.str:
	db "Memory discovery failure", 10, 0



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



;di = ptr
heap_set_ptr:
	mov [data.heap], di
	ret



;stack (callee-poped)
; uint16_t allocation size
;return AX = memory ptr
linear_alloc:
	mov ax, [data.heap]
	add ax, [esp + 4]
	mov [data.heap], ax
	sub ax, [esp + 4]
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
;destroys ax, bx, cx
put32X:

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



loader_end:
