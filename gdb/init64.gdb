
define nskip
	br *($rip+$arg0)
	c
	del br $bpnum
end

layout src
focus cmd

add-symbol-file kernel/kernel.elf

break kmain
c
del bp $bpnum
