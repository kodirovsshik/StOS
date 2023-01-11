
extern loader_end

global heap_get_ptr
global heap_set_ptr
global c_heap_get_ptr
global c_heap_set_ptr
global linear_alloc



SECTION .data
data:
	.heap dw loader_end



SECTION .text
BITS 16


;return eax = heap ptr
c_heap_get_ptr:
	xor eax, eax
	mov ax, [data.heap]
	o32 ret



;return ax = heap ptr
heap_get_ptr:
	mov ax, [data.heap]
	ret



;di = ptr
heap_set_ptr:
	mov [data.heap], di
	ret



;cdecl args:
;	void* heap ptr
c_heap_set_ptr:
	mov eax, [esp + 4]
	mov [data.heap], ax
	o32 ret



;cdecl args:
; uint16_t allocation size
;return: EAX = memory ptr
c_linear_alloc:
linear_alloc:
	mov ax, [data.heap]
	add ax, [esp + 4]
	mov [data.heap], ax
	sub ax, [esp + 4]
	shl eax, 16
	shr eax, 16
	o32 ret
