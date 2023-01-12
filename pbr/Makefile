
override PBR_BIN := ../result/pbr.bin
override PBR_SRC := src/pbr.asm

$(PBR_BIN): $(PBR_SRC)
	$(NASM) $^ -o $@ -f bin
	chmod a-x $@

clean:
