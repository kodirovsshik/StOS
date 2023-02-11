
BITS 64

global _halt

extern kmain
extern kinit



SECTION .text


kernel_entry:
	jmp kinit_asm


_halt:
	cli
	hlt
	jmp _halt




%define CR0_PE (1 << 0)
%define CR0_WP (1 << 16)
%define CR0_PG (1 << 31)

%define CR4_PAE (1 << 5)
%define CR4_PGE (1 << 7)
%define CR4_OSFXSR (1 << 9)

;TODO: consider support for:
;%define CR4_MCE (1 << 6) ;CPUID_1.EDX[7]
;%define CR4_UMIP (1 << 11) ;CPUID_7_0.ECX[2]
;%define CR4_FSGSBASE (1 << 16) ;CPUID_7_0.EBX[0]
;%define CR4_OSXSAVE (1 << 18) ;CPUID_1.ECX[26]
;%define CR4_SMEP (1 << 20) ;CPUID_7_0.EBX[7]
;%define CR4_SMAP (1 << 21) ;CPUID_7_0.EBX[20]



SECTION .init_text

kinit_asm:
	mov eax, CR0_PE | CR0_WP | CR0_PG
	mov cr0, rax

	mov eax, CR4_PAE | CR4_OSFXSR | CR4_PGE
	mov cr4, rax

	call kinit
	jmp kmain
