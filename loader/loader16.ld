
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

SECTIONS 
{
	.text :
	{
		tmp/loader.asm.elf(.text)
		*(.text)
	}
}
