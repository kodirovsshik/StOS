
extern panic2
extern puts

global do_subtask_cpu



SECTION .text
BITS 16


do_subtask_cpu:
;Perform CPU discovery with Intel's algorithm
;The CPU must support long mode
;If unsuitable CPU is detected, panic and don't return

	;jmp .check_done ;Uncomment to skip CPU discovery
	cli
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
	je err_old_cpu ;8086/80186 detected

;Assume at least 80286
	or cx, 0xF000
	push cx
	popf
	pushf
	pop ax
	and ax, 0xF000
	;I don't understand this one, why would all of them be 0 only on 80286?
	jz err_old_cpu ;80286 detected

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
	je err_old_cpu ;80386 detected

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
	je err_old_cpu ;couldn't flip CPUID flag

;CPUID present
	mov eax, 0x80000000
	cpuid
	mov ebx, 0x80000001
	cmp eax, ebx ;need extended page 1 for long mode
	jb err_old_cpu

	mov eax, ebx
	cpuid

	test edx, 1 << 29 ;long mode bit
	jz err_old_cpu

	mov si, rodata.str_nx
	test edx, 1 << 20
	jz err_cpu_feature_missing

;Long mode present
	mov eax, 1
	cpuid

	mov si, rodata.str_pae
	test edx, 1 << 6
	jz err_cpu_feature_missing

	mov si, rodata.str_apic
	test edx, 1 << 9
	jz err_cpu_feature_missing

	mov si, rodata.str_pge
	test edx, 1 << 13
	jz err_cpu_feature_missing

	mov si, rodata.str_cmov
	test edx, 1 << 15
	jz err_cpu_feature_missing

	mov si, rodata.str_mmx
	test edx, 1 << 23
	jz err_cpu_feature_missing

	mov si, rodata.str_osfxsr
	test edx, 1 << 24
	jz err_cpu_feature_missing

	mov si, rodata.str_sse2
	test edx, 1 << 25 ;SSE
	jz err_cpu_feature_missing
	test edx, 1 << 26 ;SSE2
	jz err_cpu_feature_missing

	mov si, rodata.str_cmpxchg16b
	test ecx, 1 << 13
	;jz err_cpu_feature_missing


.cleanup:
	call restore_flags

.check_done:
	movzx esp, sp
	movzx ebp, bp

	ret



;Flags are all messed up by discovery algorithm
restore_flags:
	push word 0x200
	popf
	ret



;noreturn
err_old_cpu:
	mov si, rodata.str_64
	;jmp err_cpu_feature_missing

;noreturn
;SI = err C string ptr
err_cpu_feature_missing:
	call restore_flags
	
	mov di, si
	mov si, rodata.str_err
	jmp panic2




SECTION .rodata
rodata:
	.str_err: db "Unsupported CPU feature: ", 0
	.str_64: db "64 bit mode", 0
	.str_apic: db "APIC", 0
	.str_cmov: db "CMOV", 0
	.str_mmx: db "MMX", 0
	.str_sse2: db "SSE2", 0
	.str_osfxsr: db "OSFXSR", 0
	.str_cmpxchg16b: db "CMPXCHG16B", 0
	.str_pae: db "CR4.PAE", 0
	.str_pge: db "CR4.PGE", 0
	.str_nx: db "CR4.NX", 0
