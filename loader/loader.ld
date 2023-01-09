
#OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

SECTIONS 
{
  PROVIDE(loader_begin = 0x600);

  . = loader_begin;
	.text :
	{
		tmp/loader.asm.elf(.text)
		*(.text)
	}
  .data :
  {
    *(.data)
  }
  .rodata :
  {
    *(.rodata)
  }

  . = ALIGN(8);

  PROVIDE(bss_begin = .);
  .bss :
  {
    *(.bss)
  }
  
  . = ALIGN(8);
  PROVIDE(loader_end = .);
  PROVIDE(bss_size_in_words = (. - bss_begin) / 2);

   /DISCARD/ 0 : { *(.note.gnu.build-id) *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) } 
}
