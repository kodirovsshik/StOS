define qq
	kill y
	q
end

define nskip
	br *($eip+$arg0)
	c
	del br $bpnum
end

define nin
	x/($arg0)i (($eip & 0xFFFF)+$cs*16)
	ni
end

define nint
	nskip 2	
end

define ncall
	nskip 6
end

define ncall16
	nskip 3
end

define ncall16p
	nskip 5
end
