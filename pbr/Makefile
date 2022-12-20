
override PBR_BIN := ../result/pbr.bin
override PBR_SRC := src/pbr.asm

override LOADER_BIN := ../result/loader.bin
override LOADER_OBJ := loader.asm.elf64

all: $(PBR_BIN) $(LOADER_BIN)

$(PBR_BIN): $(PBR_SRC) $(LOADER_BIN)
	$(NASM) $(PBR_SRC) -o $@_
	set -e ;\
	LOADER_SIZE=$$(stat -c "%s" $(LOADER_BIN)) ;\
	LOADER_SECTORS=$$((($$LOADER_SIZE + 511) / 512)) ;\
	../result/binecho $$LOADER_SECTORS | dd of=$@_ bs=1 count=2 seek=92 conv=notrunc
	../result/binecho 2049 | dd of=$@_ bs=1 count=8 seek=98 conv=notrunc
	mv $@_ $@

%.asm.elf64: src/%.asm
	$(NASM) $< -f elf64 -o $@ -F stabs

$(LOADER_BIN): $(LOADER_OBJ) loader.ld
	$(LINK_TARGET) $(LOADER_OBJ) -o $@.elf -T loader.ld
	$(OBJCOPY_TARGET) $@.elf $@ -O binary 

clean:
	rm -f *.elf*
