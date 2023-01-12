
override KERNEL_BIN := ../result/kernel.bin
override KERNEL_ELF = $(shell echo $(KERNEL_BIN) | sed s/.bin/.elf/g)

override KERNEL_OBJ :=
override KERNEL_OBJ += $(patsubst src/%.asm,tmp/%.asm.elf,$(wildcard src/*.asm))
override KERNEL_OBJ += $(patsubst src/%.cpp,tmp/%.cpp64.elf,$(wildcard src/*.cpp))

override KERNEL_CXX_ARGS := $(CXX_OTIME)



all:
	mkdir -p tmp
	$(MAKE) $(KERNEL_BIN)

tmp/%.asm.elf: src*/%.asm
	$(NASM) $< -f elf64 -o $@ -F dwarf

tmp/%.cpp64.elf: src*/%.cpp
	$(CXX64) $< -o $@ $(KERNEL_CXX_ARGS)

$(KERNEL_BIN): kernel.ld $(KERNEL_OBJ)
	$(LINKER_FOR_TARGET) $(KERNEL_OBJ) -o $(KERNEL_ELF) -T kernel.ld
	$(OBJCOPY_TARGET) -O binary $(KERNEL_ELF) $@
	chmod a-x $@ $(KERNEL_ELF)

clean:
	rm -rf tmp/
