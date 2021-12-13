/*
	  This file is a part of StOS project - a small operating system
	    made for learning purposes
	  Copyright (C) 2021 Egorov Stanislav, kodirovsshik@mail.ru, kodirovsshik@gmail.com
	  Copyright (C) 2014-2021 Free Software Foundation, Inc.

	  This program is free software: you can redistribute it and/or modify
	  it under the terms of the GNU General Public License as published by
	  the Free Software Foundation, either version 3 of the License, or
	  (at your option) any later version.

	  This program is distributed in the hope that it will be useful,
	  but WITHOUT ANY WARRANTY; without even the implied warranty of
	  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	  GNU General Public License for more details.

	  You should have received a copy of the GNU General Public License
	  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)

ENTRY(main)

SECTIONS
{

	PROVIDE(MBR_VERSION = 0x0100);

	. = 0x400;
	.text0 :
	{
		*(.boot_sector_text)
	}


	PROVIDE(__bootloader_start = .);
	
	.text :
	{
		*(.text)
	}

	. = ALIGN(., 16);
	.data : { *(.data) }

	. = ALIGN(., 16);
	.rodata : { *(.rodata) }

	. = ALIGN(., 16);
	.bss : { *(.bss) }

	.init_array :
	{
		PROVIDE(__init_arr = .); 
		KEEP(*(SORT_NONE(.init_array)))
		PROVIDE(__init_arr_end = .);
	}
	.fini :
	{
		PROVIDE(__fini_arr = .); 
		KEEP(*(SORT_NONE(.fini)))
		PROVIDE(__fini_arr_end = .);
	}


	PROVIDE(__bootloader_end = .);

	PROVIDE(STACK_SIZE = 4096);
	PROVIDE(STACK_TOP = ALIGN(__bootloader_end + STACK_SIZE, 4));

	PROVIDE(__bootloader_size = __bootloader_end - __bootloader_start);
	PROVIDE(__bootloader_size_sectors = ALIGN(__bootloader_size, 512) / 512);



	/* Debugging information script part was taken from default i686-elf-ld script */
	/* Copyright (C) 2014-2021 Free Software Foundation, Inc.
   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  .gnu.build.attributes : { *(.gnu.build.attributes .gnu.build.attributes.*) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1.  */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions.  */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2.  */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2.  */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions.  */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* DWARF 3.  */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }
  /* DWARF 5.  */
  .debug_addr     0 : { *(.debug_addr) }
  .debug_line_str 0 : { *(.debug_line_str) }
  .debug_loclists 0 : { *(.debug_loclists) }
  .debug_macro    0 : { *(.debug_macro) }
  .debug_names    0 : { *(.debug_names) }
  .debug_rnglists 0 : { *(.debug_rnglists) }
  .debug_str_offsets 0 : { *(.debug_str_offsets) }
  .debug_sup      0 : { *(.debug_sup) }
  .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
}
