
include ../Makefile.inc

override LOADER_BIN := ../result/loader.bin
override LOADER_ELF16 := tmp/loader.elf16
override LOADER_ELF32 := tmp/loader.elf32

override LOADER_OBJ16 :=
override LOADER_OBJ16 += tmp/a20.asm.elf
override LOADER_OBJ16 += tmp/alloc.asm.elf
override LOADER_OBJ16 += tmp/conio.asm.elf
override LOADER_OBJ16 += tmp/cpu.asm.elf
override LOADER_OBJ16 += tmp/debug.asm.elf
override LOADER_OBJ16 += tmp/farptr.asm.elf
override LOADER_OBJ16 += tmp/interrupt.asm.elf
override LOADER_OBJ16 += tmp/kbio.asm.elf
override LOADER_OBJ16 += tmp/loader.asm.elf
override LOADER_OBJ16 += tmp/memfunc.cpp16.elf
override LOADER_OBJ16 += tmp/ram.cpp16.elf
override LOADER_OBJ16 += tmp/time.asm.elf
override LOADER_OBJ16 += tmp/vbe.cpp16.elf

override LOADER_OBJ32 :=
override LOADER_OBJ32 += tmp/pm.asm.elf

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

$(LOADER_ELF16): $(LOADER_OBJ16) loader16.ld
	$(LINKER_FOR_TARGET) -r -o $@ $(LOADER_OBJ16) -T loader16.ld
	$(OBJCOPY_TARGET) -w -L \* $@ $@

$(LOADER_ELF32): $(LOADER_OBJ32)
	$(LINKER_FOR_TARGET) -Wl,-melf_i386 -r -o $@ $(LOADER_OBJ32)
	$(OBJCOPY_TARGET) -w -L \* $@ $@
	$(OBJCOPY_TARGET) --globalize-symbol=go_pm $@ $@

$(LOADER_BIN): $(LOADER_ELF16) $(LOADER_ELF32) loader.ld
	$(LINKER_FOR_TARGET) $(LOADER_ELF16) $(LOADER_ELF32) -o $@.elf -T loader.ld -Wl$(GNU_LD_RWX_SEGMENT_WARNING),--build-id=none
	$(OBJCOPY_TARGET) $@.elf $@ -O binary
	ndisasm -o 0x600 $@ > $$(echo $@ | sed s/.bin/_disasm/g)

clean:
	rm -rf tmp/
