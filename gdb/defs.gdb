
define nskip
	br *($eip+$arg0)
	c
	del br $bpnum
end
