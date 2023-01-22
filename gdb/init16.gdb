
define nint
	nskip 2	
end

#16 bit call in 16 bit code
define nc
	nskip 3
end

#32 bit call in 16 bit code
define nc32
	nskip 6
end

set tdesc filename gdb/target16.xml
set architecture i8086

add-symbol-file loader/loader.elf

layout src
layout regs
focus cmd

break loader_main.loader_end

break loader_main
c
del bp $bpnum
