
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

target remote localhost:1234

add-symbol-file result/loader.elf
add-symbol-file result/kernel.elf

break loader_main.loader_end

break pm_main
#break loader_main
break *0x600
c
del bp $bpnum
