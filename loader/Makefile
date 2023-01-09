
include ../Makefile.inc

override LOADER_BIN := ../result/loader.bin
override LOADER_OBJ :=
override LOADER_OBJ += tmp/a20.asm.elf
override LOADER_OBJ += tmp/alloc.asm.elf
override LOADER_OBJ += tmp/conio.asm.elf
override LOADER_OBJ += tmp/cpu.asm.elf
override LOADER_OBJ += tmp/debug.asm.elf
override LOADER_OBJ += tmp/farptr.asm.elf
override LOADER_OBJ += tmp/interrupt.asm.elf
override LOADER_OBJ += tmp/kbio.asm.elf
override LOADER_OBJ += tmp/loader.asm.elf
override LOADER_OBJ += tmp/memfunc.cpp16.elf
override LOADER_OBJ += tmp/ram.cpp16.elf
override LOADER_OBJ += tmp/time.asm.elf
override LOADER_OBJ += tmp/vbe.cpp16.elf

override GNU_LD_RWX_SEGMENT_WARNING := \
$(shell (echo "SECTIONS{}" | \
	$(LINKER_FOR_TARGET) /dev/null -T /dev/stdin -nostartfiles -Wl,--no-warn-rwx-segment 2>/dev/null -o /dev/zero) \
&& echo ,--no-warn-rwx-segment || echo)



all:
	mkdir -p tmp
	$(MAKE) $(LOADER_BIN)

tmp/%.asm.elf: src/%.asm
	$(NASM) $< -f elf32 -o $@ -F stabs

tmp/%.cpp16.elf: src/%.cpp
	$(CXX16) $< -o $@ $(call get_osize,$(CXX16)) -mgeneral-regs-only -march=i486 -Wno-return-type-c-linkage
#$(OBJCOPY_TARGET) -O elf64-x86-64 $@ $@

$(LOADER_BIN): $(LOADER_OBJ) loader.ld
	$(LINKER_FOR_TARGET) $(LOADER_OBJ) -m32 -o $@.elf -T loader.ld -Wl$(GNU_LD_RWX_SEGMENT_WARNING),--build-id=none
	$(OBJCOPY_TARGET) $@.elf $@ -O binary
	ndisasm -o 0x600 $@ > $$(echo $@ | sed s/.bin/_disasm/g)

clean:
	rm -rf tmp/
