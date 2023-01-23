
define nskip
	br *($eip+$arg0)
	c
	del br $bpnum
end

layout src
focus cmd

set architecture i386

add-symbol-file loader/loader.elf

break pm_main
c
del bp $bpnum
