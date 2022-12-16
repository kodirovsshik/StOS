set disassembly-flavor intel
set tdesc filename gdb/target32.xml
set architecture i8086

target remote localhost:1234

break *0x7c00
c
si

tui enable
layout asm
layout regs
focus cmd

