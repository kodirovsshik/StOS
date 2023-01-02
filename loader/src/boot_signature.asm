
%define CMOS_SECONDS 0x00
%define CMOS_MINUTES 0x02
%define CMOS_HOURS 0x04
%define CMOS_DAY 0x07
%define CMOS_MONTH 0x08
%define CMOS_YEAR 0x09

;PBR has a boot signature field in it
;It is to be written by loader
;and the PBR is them to be written to disk
;and found be the kernel
%define PBR_BOOT_SIG_FIELD_ADDR (0x7C00 + 432)

extern panic
extern puts
extern put32x
extern putNx
extern endl
extern bss.pbr_disk
extern edata.pbr_lba
extern edata.boot_signature

global create_boot_signature



SECTION .rodata
rodata:
	.str_boot_signature_ok1: db "Written boot signature ", 0
	.str_boot_signature_ok2: db " to disk ", 0
	.str_boot_signature_ok3: db " at lba ", 0



SECTION .data
pbr_lba_packet:
	db 16
	db 0
	dw 1
	dd 0x00007C00
.lba:
	dq 0



SECTION .text
BITS 16


create_boot_signature:
	mov di, PBR_BOOT_SIG_FIELD_ADDR
	xor bx, bx

.create_low_dword:
	;boot signature low dword's CMOS regs:
	;00: seconds
	;02: minutes
	;04: hours

	xor eax, eax

	mov al, CMOS_HOURS
	call cmos_read
	shl ax, 8

	mov al, CMOS_MINUTES
	call cmos_read
	shl eax, 8

	mov al, CMOS_SECONDS
	call cmos_read

	stosd
	mov [edata.boot_signature], eax

.create_high_dword:
	;boot signature high dword's CMOS regs:
	;07: day
	;08: month
	;09: year
	
	xor eax, eax

	mov al, CMOS_YEAR
	call cmos_read
	shl ax, 8

	mov al, CMOS_MONTH
	call cmos_read
	shl eax, 8

	mov al, CMOS_DAY
	call cmos_read

	stosd
	mov [edata.boot_signature + 4], eax

.store_lba:
	mov si, edata.pbr_lba
	mov di, pbr_lba_packet.lba
	times 2 movsd

.write_pbr:
	mov si, pbr_lba_packet
	mov dl, [bss.pbr_disk]
	mov ax, 0x4300
	int 0x13
	jc err_boot_signature_write
	test ah, ah
	jnz err_boot_signature_write

.report_results:
	mov si, rodata.str_boot_signature_ok1
	call puts
	mov eax, [edata.boot_signature + 4]
	call put32x
	mov eax, [edata.boot_signature]
	call put32x

	mov si, rodata.str_boot_signature_ok2
	call puts
	movzx eax, byte [bss.pbr_disk]
	mov cx, 2
	call putNx

	mov si, rodata.str_boot_signature_ok3
	call puts

	mov eax, [pbr_lba_packet.lba + 4]
	call put32x
	mov eax, [pbr_lba_packet.lba]
	call put32x

	call endl

.done:
	ret



;[in] al = register
;[out] al = data
cmos_read:
	pushf
	cli

	and al, ~0x80 ;clear the NMI DISABLE bit
	out 0x70, al
	in al, 0x71
	
	popf
	ret



;noreturn
err_boot_signature_write:
	mov si, .str
	jmp panic
.str:
	db "Boot disk write error", 0
