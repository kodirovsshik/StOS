
tui enable
layout src
layout regs
focus cmd

set disassembly-flavor intel

display/x { $eax, $ebx, $ecx, $edx, $esi, $edi, $ebp, $esp }
display/x { $cs, $ds, $ss, $es, $fs, $gs }
