
define nint
	nskip 2	
end

define ncall
	nskip 3
end

set disassembly-flavor intel
set tdesc filename gdb/target32.xml
set architecture i8086

target remote localhost:1234

symbol-file ./result/loader.bin.elf

break halt
break loader_main.prehalt

break loader_main.vbe_print_modes
break vbe_print_video_modes
c
