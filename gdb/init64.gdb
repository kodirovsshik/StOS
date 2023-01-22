
layout src
focus cmd

set architecture i386

add-symbol-file loader/loader.elf
add-symbol-file kernel/kernel.elf

layout src
focus cmd

break go_lm
c
del bp $bpnum
