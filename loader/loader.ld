
/* ENTRY(_start) */

SECTIONS 
{
  . = 0x600;
	.text :
	{
		tmp/loader.asm.elf64(.text)
		*(.text)
	}
  .data :
  {
    *(.data)
    *(.rodata)
  }

  . = ALIGN(8);

  PROVIDE(bss_begin = .);
  .bss :
  {
    *(.bss)
  }
  
  . = ALIGN(4);
  PROVIDE(loader_end = .);
  PROVIDE(bss_size_w = (. - bss_begin) / 2);


  /* Stabs debugging sections.  */
  .stab         0 : { *(.stab) }
  .stabstr      0 : { *(.stabstr) }
  .stab.excl    0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index   0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment      0 : { *(.comment) }
  .gnu.build.attributes0 : { *(.gnu.build.attributes .gnu.build.attributes.*) }

   /DISCARD/ 0 : { *(.note.gnu.build-id) *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) } 
}
