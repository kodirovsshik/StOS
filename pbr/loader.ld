
/* ENTRY(_start) */

SECTIONS 
{
  . = 0x600;
	.text :
	{
		*(.text)
	}

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
