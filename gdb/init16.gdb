
define nint
	nskip 2	
end

#call 16 bit from 16 bit code
define nc
	nskip 3
end

#call 32 bit from 16 bit code
define nc32
	nskip 6
end

set disassembly-flavor intel
set tdesc filename gdb/target32.xml
set architecture i8086

target remote localhost:1234

add-symbol-file result/loader.bin.elf

break loader_main.prehalt

#break loader_main
c
del bp $bpnum
