
override MBR_BIN := ../result/mbr.bin
override MBR_SRC := src/mbr.asm

$(MBR_BIN): $(MBR_SRC)
	$(NASM) $^ -o $@ -f bin
	chmod a-x $@

clean:
