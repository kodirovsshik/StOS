
include ../Makefile.inc

override LOADER_BIN := ../result/loader.bin
override LOADER_ELF = $(shell echo $(LOADER_BIN) | sed s/.bin/.elf/g)
override LOADER_DISASM = $(shell echo $(LOADER_BIN) | sed s/.bin/_disasm/g)

override LOADER_ELF16 := tmp/loader.elf16
override LOADER_ELF32 := tmp/loader.elf32

override LOADER_OBJ16 :=
override LOADER_OBJ16 += $(patsubst src16/%.asm,tmp/%.asm.elf,$(wildcard src16/*.asm))
override LOADER_OBJ16 += $(patsubst src16/%.cpp,tmp/%.cpp16.elf,$(wildcard src16/*.cpp))

override LOADER_OBJ32 :=
override LOADER_OBJ32 += $(patsubst src32/%.asm,tmp/%.asm.elf,$(wildcard src32/*.asm))
override LOADER_OBJ32 += $(patsubst src32/%.cpp,tmp/%.cpp32.elf,$(wildcard src32/*.cpp))


override GNU_LD_RWX_SEGMENT_WARNING = \
$(shell (echo "SECTIONS{}" | \
	$(LINKER_FOR_TARGET) /dev/null -T /dev/stdin -nostartfiles -Wl,--no-warn-rwx-segment 2>/dev/null -o /dev/zero) \
&& echo ,--no-warn-rwx-segment || echo)

override LOADER_CXX_ARGS := \
	-mgeneral-regs-only \
	-march=i486 \
	-Wno-return-type-c-linkage \
	-I.



all:
	mkdir -p tmp
	$(MAKE) $(LOADER_BIN)

tmp/%.asm.elf: src*/%.asm
	$(NASM) $< -f elf32 -o $@ -F stabs

tmp/%.cpp16.elf: src*/%.cpp
	$(CXX16) $< -o $@ $(call get_osize,$(CXX16)) $(LOADER_CXX_ARGS)

tmp/%.cpp32.elf: src*/%.cpp
	$(CXX32) $< -o $@ $(call get_osize,$(CXX32)) $(LOADER_CXX_ARGS)

$(LOADER_ELF16): $(LOADER_OBJ16) loader16.ld
	$(LINKER_FOR_TARGET) -r -o $@ $(LOADER_OBJ16) -T loader16.ld
	$(OBJCOPY_TARGET) -w -L \* $@ $@

$(LOADER_ELF32): $(LOADER_OBJ32)
	$(LINKER_FOR_TARGET) -Wl,-melf_i386 -r -o $@ $(LOADER_OBJ32)
	$(OBJCOPY_TARGET) -w -L \* $@ $@
	$(OBJCOPY_TARGET) --globalize-symbol=go_pm $@ $@

$(LOADER_BIN): $(LOADER_ELF16) $(LOADER_ELF32) loader.ld
	$(LINKER_FOR_TARGET) $(LOADER_ELF16) $(LOADER_ELF32) -o $(LOADER_ELF) -T loader.ld -Wl$(GNU_LD_RWX_SEGMENT_WARNING),--build-id=none
	$(OBJCOPY_TARGET) $(LOADER_ELF) $@ -O binary
	ndisasm -o 0x600 $@ > $(LOADER_DISASM)

clean:
	rm -rf tmp/
