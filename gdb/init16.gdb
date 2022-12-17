
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

break *0x70A
c
si
