
override MBR_BIN := ../result/mbr.bin
override MBR_SRC := src/mbr.asm

$(MBR_BIN): $(MBR_SRC)
	$(NASM) $(MBR_SRC) -o $(MBR_BIN) -f bin

clean:
