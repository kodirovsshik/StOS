
tui enable
#layout split
layout asm
layout regs
focus cmd

display/x { $eax, $ebx, $ecx, $edx, $esi, $edi, $ebp, $esp }
display/x { $cs, $ds, $ss, $es, $fs, $gs }
