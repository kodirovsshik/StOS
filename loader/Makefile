
override PBR_BIN := ../result/pbr.bin
override PBR_SRC := src/pbr.asm

override LOADER_BIN := ../result/loader.bin
override LOADER_OBJ :=
override LOADER_OBJ += tmp/a20.asm.elf
override LOADER_OBJ += tmp/alloc.asm.elf
override LOADER_OBJ += tmp/boot_signature.asm.elf
override LOADER_OBJ += tmp/conio.asm.elf
override LOADER_OBJ += tmp/cpu.asm.elf
override LOADER_OBJ += tmp/farptr.asm.elf
override LOADER_OBJ += tmp/interrupt.asm.elf
override LOADER_OBJ += tmp/kbio.asm.elf
override LOADER_OBJ += tmp/loader.asm.elf
override LOADER_OBJ += tmp/loader.cpp16.elf
override LOADER_OBJ += tmp/ram.cpp16.elf
override LOADER_OBJ += tmp/time.asm.elf
override LOADER_OBJ += tmp/vbe.cpp16.elf

override GNU_LD_RWX_SEGMENT_WARNING := \
$$( (echo "SECTIONS{}" | \
	$(LINKER_FOR_TARGET) /dev/null -T /dev/stdin -nostartfiles -Wl,--no-warn-rwx-segment 2>/dev/null) \
&& echo ,--no-warn-rwx-segment || echo)


all: $(PBR_BIN) $(LOADER_BIN)

$(PBR_BIN): $(PBR_SRC) $(LOADER_BIN)
	$(NASM) $(PBR_SRC) -o $@_
	set -e ;\
	LOADER_SIZE=$$(stat -c "%s" $(LOADER_BIN)) ;\
	LOADER_SECTORS=$$((($$LOADER_SIZE + 511) / 512)) ;\
	../result/binecho $$LOADER_SECTORS | dd of=$@_ bs=1 count=2 seek=92 conv=notrunc
	../result/binecho 2049 | dd of=$@_ bs=1 count=8 seek=98 conv=notrunc
	mv $@_ $@

tmp/%.asm.elf: src/%.asm
	mkdir -p tmp
	$(NASM) $< -f elf32 -o $@ -F stabs

tmp/%.cpp16.elf: src/%.cpp
	mkdir -p tmp
	$(CXX16) $< -o $@ -mgeneral-regs-only $(CXX_OSIZE) -march=i486 -Wno-return-type-c-linkage
	#$(OBJCOPY_TARGET) -O elf64-x86-64 $@ $@

$(LOADER_BIN): $(LOADER_OBJ) loader.ld
	$(LINKER_FOR_TARGET) $(LOADER_OBJ) -m32 -o $@.elf -T loader.ld -Wl$(GNU_LD_RWX_SEGMENT_WARNING),--build-id=none
	$(OBJCOPY_TARGET) $@.elf $@ -O binary
	ndisasm -o 0x600 $@ > $$(echo $@ | sed s/.bin/_disasm/g)

clean:
	rm -rf tmp/
