
SECTION .text
BITS 16

global heap_get_ptr
global heap_set_ptr
global linear_alloc

extern data.heap



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
