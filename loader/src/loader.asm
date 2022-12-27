
global data.heap
global data.has_memory_over_1m
global edata.memory_map_addr
global edata.memory_map_size
global edata.memory_at_1m
global edata.memory_at_16m
global edata.e820_ok
global rodata.str_mem1
global rodata.str_mem2
global rodata.str_vbe
global rodata.str_vbe_modes1
global rodata.str_vbe_modes2
global edata.vbe_modes_ptr
global edata.vbe_modes_count
global data.vbe_modes_count_reported
global halt

extern puts
extern check_cpu
extern check_memory
extern check_vbe
extern activate_a20
extern loader_end

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
	.heap dw 0x600
	.has_memory_over_1m db 0
	db 0
	.vbe_modes_count_reported dw 0

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



loader_main:

.setup_stack:
	cli

	xor ax, ax
	mov ds, ax
	mov es, ax

	mov sp, ax
	mov ax, 0x7000
	mov ss, ax

	sti

.logo:
	mov ax, 0x0002
	int 0x10

	mov si, rodata.str_logo
	call puts

.setup_heap:
	mov ax, loader_end + 3
	and ax, ~3
	mov [data.heap], ax

.check_cpu:
	cli
	call check_cpu

.check_memory:
	call check_memory

	cmp byte [data.has_memory_over_1m], 1
	jne .a20_done

.a20:
	call activate_a20
.a20_done:
	nop

.dump_bios_ports_data:
	mov si, 0x400
	mov di, edata.io_ports
	mov cx, 7
	rep movsw

.vbe:
	call check_vbe

.prehalt:
	jmp halt



halt:
	mov si, .str
	call puts
.x:
	hlt
	jmp .x
.str:
	db 10, "CPU HALTED", 10, 0
