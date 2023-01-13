
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

SECTIONS
{
	. = 0xFFFFFFFF80000000; /* -2 GiB */
	PROVIDE(_kernel_image_begin = .);

	PROVIDE(_kernel_text_begin = .);
	.text :
	{
		*kentry.asm.elf(.text)
		*(.text*)
	}

	. = ALIGN(4096);
	PROVIDE(_kernel_text_end = .);
	PROVIDE(_kernel_data_begin = .);
	PROVIDE(_kernel_rwdata_begin = .);

	.data :
	{
		*(.data*)
	}

	. = ALIGN(8);
	PROVIDE(_kernel_data_end = .);
	PROVIDE(_kernel_bss_begin = .);

	.bss (NOLOAD) :
	{
		*(.bss*) *(COMMON)
	}

	. = ALIGN(4096);
	PROVIDE(_kernel_rwdata_end = .);
	PROVIDE(_kernel_bss_end = .);
	PROVIDE(_kernel_rodata_begin = .);

	.rodata :
	{
		*(.rodata*)
	}

	. = ALIGN(4096);
	PROVIDE(_kernel_rodata_end = .);
	PROVIDE(_kernel_image_end = .);
	
	
   /DISCARD/ 0 : { *(.note.gnu.build-id) *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) } 
}
