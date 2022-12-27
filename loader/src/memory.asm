
extern halt
extern puts
extern endl
extern put32u
extern put32x
extern linear_alloc
extern heap_get_ptr
extern heap_set_ptr
extern data.has_memory_over_1m
extern edata.memory_map_addr
extern edata.memory_map_size
extern space
extern edata.memory_at_1m
extern edata.memory_at_16m
extern edata.e820_ok
extern rodata.str_mem1
extern rodata.str_mem2

global check_memory

SECTION .text
BITS 16

extern err_memory_detection

check_memory:

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
	call set_low_memory_size

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
set_low_memory_size:
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



try_fill_memory_table_e820:
	mov bp, sp
	and sp, 0xFFFC

	sub sp, 24
	mov di, sp

	xor ebx, ebx
	mov eax, 0x0000E820
	mov ecx, 24
	mov edx, 0x534D4150
	int 0x15

	jc .ret0
	cmp eax, 0x534D4150
	jne .ret0

	call heap_get_ptr
	mov [edata.memory_map_addr], ax

	xor ebx, ebx
.loop:
	mov eax, 0x0000E820
	mov ecx, 24
	mov edx, 0x534D4150
	push es
	push ss
	pop es
	int 0x15
	pop es
	jc .ret1
	pushf

	call .insert

	popf
	;call .print

	test ebx, ebx
	jz .ret1

	;call wait_enter_hit

	jmp .loop


.ret1:
	mov di, [edata.memory_map_addr]
	call heap_set_ptr
	mov ax, [edata.memory_map_size]
	shl ax, 3
	sub di, ax
	shl ax, 1
	sub di, ax
	mov [edata.memory_map_addr], di 
	clc
	jmp .ret
.ret0:
	stc
.ret:
	mov sp, bp
	ret
;of all the things, lambdas are literally the easiest thing in assembly
;needs all gp regs as set after the interrupt
;preserves all gp registers
.insert:
	pushad
	
	cmp cx, 20
	ja .insert.after_acpi3
	mov dword ss:[di + 20], 1
.insert.after_acpi3:
	test dword ss:[di + 20], 1
	jz .insert.done
	test dword ss:[di + 20], 2
	jnz .insert.done

	mov ecx, ss:[di + 0]
	mov edx, ss:[di + 4]
	add ecx, ss:[di + 8]
	adc edx, ss:[di + 12]
	mov ss:[di + 8], ecx
	mov ss:[di + 12], edx

	test edx, edx
	jnz .insert.set_1m_flag
	cmp ecx, 0x100000
	jbe .insert.skip_set_1m_flag

.insert.set_1m_flag:
	mov byte [data.has_memory_over_1m], 1

.insert.skip_set_1m_flag:
	push ds

	push ss
	pop ds
	mov si, di

	;es=0
	mov di, es:[edata.memory_map_addr]
	
	mov cx, 6
	rep movsd
	
	pop ds

	mov [edata.memory_map_addr], di
	inc word [edata.memory_map_size]

.insert.done:
	popad
	ret

;needs all gp regs as set after the interrupt
;preserves all gp registers
.print:
	pushad
	pushfd

	push ebx ;save for later

	;print returned buffer size after call
	mov eax, ecx
	call put32u
	call space

	;print next cell value after call
	pop eax ;ebx value
	call put32u
	call space

	;print entry base address
	mov eax, ss:[di + 4]
	call put32x
	mov eax, ss:[di + 0]
	call put32x
	call space

	;print entry size
	mov eax, ss:[di + 12]
	call put32x
	mov eax, ss:[di + 8]
	call put32x
	call space

	;print type
	mov eax, ss:[di + 16]
	call put32u
	call space

	;print ACPI 3.0 flags
	mov eax, ss:[di + 20]
	call put32x

	call endl

	popfd
	popad
	ret



;return:
;eax = memory size in KiB
;destroys eax, bx, cx, edx
get_memory_size_KiB:
	xor eax, eax
	mov si, [edata.memory_map_addr]
	mov cx, [edata.memory_map_size]

.l1:
	cmp dword [si + 16], 1
	jne .next
	test dword [si + 20], 1
	jz .next
	
	mov edx, [si + 8]
	mov ebx, [si + 12]
	sub edx, [si + 0]
	sbb ebx, [si + 4]

	add edx, 512
	pushf
	
	shr edx, 10
	add eax, edx
	mov edx, [si + 12]
	
	popf
	
	adc ebx, 0
	shl ebx, 22
	add eax, ebx

.next:
	add si, 24
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



err_memory_detection:
	mov si, .str
	call puts
	jmp halt
.str:
	db "Memory discovery failure", 10, 0
