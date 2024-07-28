
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

SECTIONS
{
	. = 0xFFFFFFFF80000000; /* -2 GiB */
	PROVIDE(_kernel_image_begin = .);



	PROVIDE(_kernel_code_begin = .);
	.text :
	{
		*kentry.asm.elf(.text)
		EXCLUDE_FILE(*kinit*) *(.text*)
	}

	PROVIDE(_kernel_code_end = .);
	. = ALIGN(4096);
	PROVIDE(_kernel_code_size_pages = (. - _kernel_code_begin) / 4096);



	PROVIDE(_kernel_data_begin = .);

	.data :
	{
		EXCLUDE_FILE(*kinit*) *(.data*)
	}

	PROVIDE(_kernel_data_end = .);
	. = ALIGN(4096);
	PROVIDE(_kernel_data_size_pages = (. - _kernel_data_begin) / 4096);



	PROVIDE(_kernel_rodata_begin = .);

	.rodata :
	{
		EXCLUDE_FILE(*kinit*) *(.rodata*)
		EXCLUDE_FILE(*kinit*) *(.eh_frame*)
	}

	PROVIDE(_kernel_rodata_end = .);
	. = ALIGN(4096);
	PROVIDE(_kernel_rodata_size_pages = (. - _kernel_rodata_begin) / 4096);



	PROVIDE(_kernel_init_section_begin = .);

	.init_section1 :
	{
		*(.init_text*)
		*kinit*(.text*)
		*kinit*(.*data*)
		*kinit*(.eh_frame*)
	}

	. = ALIGN(4096);



	PROVIDE(_kernel_bss_begin = .);

	.init_section_bss (NOLOAD) :
	{
		*kinit*(.bss*)
	}

	. = ALIGN(16);
	PROVIDE(_kernel_init_section_end = .);
	. = ALIGN(4096);

	.bss (NOLOAD) :
	{
		EXCLUDE_FILE(*kinit*) *(.bss*) *(COMMON)
	}

	. = ALIGN(16);
	PROVIDE(_kernel_bss_end = .);
	. = ALIGN(4096);
	PROVIDE(_kernel_bss_size_pages = (. - _kernel_bss_begin) / 4096);



	PROVIDE(_kernel_image_end = .);
	
	
   /DISCARD/ 0 : { *(.comment*) *(.note*) *(.gnu_debuglink) *(.gnu.lto_*) } 
}
