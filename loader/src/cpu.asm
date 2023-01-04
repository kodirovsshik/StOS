
extern panic
extern puts
extern endl

global do_subtask_cpu



SECTION .text
BITS 16


do_subtask_cpu:
;Perform CPU discovery with Intel's algorithm
;The CPU must support long mode
;If unsuitable CPU is detected, panic and don't return

	jmp .check_done ;Uncomment to skip CPU discovery
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
	je err_unsupported_cpu ;8086/80186 detected

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

;Supported CPU detected, CPU discovery done
.cleanup:
	;Flags are all messed up by discovery algorithm
	push dword 0x200
	popfd ;IF=1

.check_done:
	movzx esp, sp
	movzx ebp, bp

	call print_cpu_data_strings
	ret



print_cpu_data_strings:
.setup:
	sub sp, 52

	mov di, sp
	mov si, sp

.get_manufacturer_string:
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
.print_manufacturer_string:
	call puts

.check_brand_string_present:
	mov eax, 0x80000000
	cpuid
	cmp eax, 0x80000004
	jb .ret

.get_brand_string:
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

.unpad_brand_string: ;why do Intel pad their CPU brand strings with spaces
	lodsb
	cmp al, ' '
	je .unpad_brand_string

.print_brand_string:
	dec si
	call puts
	call endl

.ret:
	add sp, 52
	ret



;noreturn
err_unsupported_cpu:
	push word 0x200
	popf
	mov si, .str
	jmp panic
.str:
	db "Unsupported CPU detected, 64 bit support required", 0
