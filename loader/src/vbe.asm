
extern putc
extern puts
extern space
extern endl
extern put32u
extern put32x
extern heap_get_ptr
extern heap_set_ptr
extern halt
extern rodata.str_vbe
extern rodata.str_vbe_modes1
extern rodata.str_vbe_modes2
extern edata.vbe_modes_count
extern data.vbe_modes_count_reported
extern edata.vbe_modes_ptr
extern wait_enter_hit

global check_vbe

SECTION .text
BITS 16


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

	call space

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

	call space

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
	inc word [es:data.vbe_modes_count_reported]
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



vbe_err_old:
	mov si, .str
	call puts
	jmp halt
.str:
	db "VBE 3.0 or newer required", 10, 0	
	
	
	
check_vbe:
	push es
	push ss
	pop es

	sub sp, 512
	mov di, sp
	mov dword [ss:di], 0x32454256 ;'VBE2'
	mov ax, 0x4F00
	int 0x10

	pop es

	cmp ax, 0x4F
	jne vbe_err_old

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

.vbe_get_modes:
	mov eax, [ss:di + 14]
	call vbe_copy_video_modes

	xor eax, eax
	mov ax, [data.vbe_modes_count_reported]
	call put32u
	mov si, rodata.str_vbe_modes1
	call puts

	xor eax, eax
	mov ax, [edata.vbe_modes_count]
	call put32u
	mov si, rodata.str_vbe_modes2
	call puts

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

	call space
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
	ret
