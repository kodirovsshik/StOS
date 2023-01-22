
layout src
focus cmd

set architecture i386

add-symbol-file loader/loader.elf

layout src
focus cmd

break pm_main
c
del bp $bpnum
