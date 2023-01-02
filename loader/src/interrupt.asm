
global interrupt
global get_ss



SECTION .text
BITS 16



;cdecl args:
;	interrupt number
;	ptr to struct with eax, ecx, edx, ebx, esi, edi
interrupt:
	push ebp
	mov ebp, esp
	pushad
	push ebp

	mov al, byte [ebp + 12]
	mov [.int_number], al

	mov ebp, [ebp + 8]

	mov eax, [ebp + 0]
	mov ecx, [ebp + 4]
	mov edx, [ebp + 8]
	mov ebx, [ebp + 12]
	mov esi, [ebp + 16]
	mov edi, [ebp + 20]
	mov ebp, [ebp + 24]

	sti
	db 0xCD
.int_number:
	db 0x00

	push ebp
	mov ebp, [ss:esp + 4] ;get the stack frame
	mov ebp, [ebp + 8] ;get the regs structure ptr
	
	mov [ebp + 0], eax
	mov [ebp + 4], ecx
	mov [ebp + 8], edx
	mov [ebp + 12], ebx
	mov [ebp + 16], esi
	mov [ebp + 20], edi
	pop dword [ebp + 24]
	pushfd
	pop dword [ebp + 28]
	
	pop ebp ;gets the stack frame
	popad
	pop ebp
	o32 ret
