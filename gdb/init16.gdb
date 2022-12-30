
define nint
	nskip 2	
end

define ncall16
	nskip 3
end

set disassembly-flavor intel
set tdesc filename gdb/target32.xml
set architecture i8086

target remote localhost:1234

symbol-file ./result/loader.bin.elf

break loader_main.prehalt

#break loader_main
break create_boot_signature.ok
c
