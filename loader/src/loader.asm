
;Here's where the real deal begins
;The plan:
;✓	Flex off with cool loading message
;✓		Just display some loading message really
;✓	Perform CPU discovery
;✓		64-bit-capable CPU is required
;	Check for available memory
;✓		Perform memory discovery
;		Check for region [1 mb; 1mb + X) to be available
;	Enable A20 pin
;✓	Calculate boot signature
;✓		Use various CMOS data to create signature of boot time
;✓		Write it to partition boot record of partition we boot from
;✓		Save it for kernel for it to be able to find root partition
;	Setup reasonable video mode with VBE
;✓		Check for VBE 3.0 support
;		Find compliant video modes (Supported, RBG, 24 bit or wider)
;		If no found, stop booting
;		Pick largest video mode not exceeding 768 in height
;		If no found, pick the one with smallest height
;	Setup protected mode environment
;		Basic temporary GDT with 32-bit code and data segments
;	Go 32 bit mode
;		By this point, we should have already been using C++
;	Setup 64-bit environment
;		Identity page mapping for usable memory
;		except for -2GB address space for the kernel
;			[0xFFFFFFFF80000000; ...) -> [0x100000; ...),
;		except for low 1 MiB
;	Go 64 bit
;	Setup -2GB address space mapping for the kernel
;	Flex off with some PCI commands
;		Probe PCI bus for storage devices
;	Load kernel and transfer the control

;Assumptions:
;	loaded at 0x0000:0x0600
;	DL = current BIOS disk number
;	qword [PBR_LBA_ADDR] = pointer to partition's 8-byte LBA

;Memory map:
;	Segment 0x0000: code & data
;		0x0600: loader
;		0x7C00: PBR
;	Segment 0x1000: stack space
;	Segment 0x2000: output log buffer
;
;Conventions:
;DS = CS = ES = 0, SS = stack segment
;	except for small places in code where we need otherwise
;

%define PBR_LBA_ADDR 0x5F8


;functions
;global halt
global panic

;variables
global edata.pbr_lba
global edata.boot_signature
global edata.memory_map_addr
global edata.memory_map_size
global edata.memory_at_1m
global edata.memory_at_16m
global edata.e820_ok
global edata.vbe_modes_ptr
global edata.vbe_modes_count
global bss.pbr_disk


;functions
extern puts
extern check_cpu
extern check_memory
extern check_vbe
extern activate_a20
extern create_boot_signature

;variables
extern data.output_use_screen

;data provided by linker
extern loader_end
extern bss_begin
extern bss_size_w



SECTION .rodata
rodata:
	.str_logo db 10, "StOS loader v1.0", 10, 0
	.str_loader_end db "Error: loader end reached", 0
	.str_panic db 10, "BOOTLOADER PANIC:", 10, 0



SECTION .bss
bss:
	.pbr_disk resb 1



SECTION .text
BITS 16

loader_begin:
	jmp loader_main

align 8, nop
edata: ;data structure to be read by kernel
	.pbr_lba dq 0
	.boot_signature dq 0
	.vbe_modes_ptr dw 0
	.vbe_modes_count dw 0
	.memory_map_addr dw 0
	.memory_map_size dw 0
	.memory_at_1m dw 0
	.memory_at_16m dw 0
	.e820_ok db 1


loader_main:
	nop

.setup_memory_layout:
	cli

	xor ax, ax
	mov ds, ax
	mov es, ax

	mov sp, ax
	mov ax, 0x1000
	mov ss, ax

	sti

.clear_bss:
	cld
	mov di, bss_begin
	mov cx, bss_size_w
	xor ax, ax
	rep stosw

.store_pbr_data:
	mov [bss.pbr_disk], dl

	mov si, PBR_LBA_ADDR
	mov di, edata.pbr_lba
	times 4 movsw

.logo:
	mov ax, 0x0002
	int 0x10

	mov si, rodata.str_logo
	call puts

.disable_screen_output:
	mov byte [data.output_use_screen], 0

.check_cpu:
	call check_cpu

.check_memory:
	call check_memory

.a20:
	call activate_a20

.boot_signature:
	call create_boot_signature

.vbe:
	call check_vbe



.prehalt:
	mov si, rodata.str_loader_end
	;jmp panic

;si = str
;noreturn
panic:
	mov byte [data.output_use_screen], 1
	sti
	
	push si
	mov si, rodata.str_panic
	call puts

	pop si
	call puts
	;jmp halt

;noreturn
halt:
	mov byte [data.output_use_screen], 1
	mov si, .str
	call puts
.x:
	hlt
	jmp .x
.str:
	db 10, "CPU HALTED", 10, 0
