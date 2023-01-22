
override LOADER_ELF = $(subst .bin,.elf,$(LOADER_BIN))
override LOADER_DISASM = $(subst .bin,.disasm,$(LOADER_BIN))
override LOADER_SRC :=

override LOADER_SRC16 := $(LOADER_DIR)/src16
override LOADER_SRC32 := $(LOADER_DIR)/src32
override LOADER_TMP := $(LOADER_DIR)/tmp
override LOADER_LAYOUT := $(LOADER_TMP)

override LOADER_ELF16 := $(LOADER_TMP)/loader.elf16
override LOADER_ELF32 := $(LOADER_TMP)/loader.elf32

override LOADER_OBJ16 :=
override LOADER_OBJ16 += $(call find_objs_names,asm,.elf,$(LOADER_SRC16),$(LOADER_TMP))
override LOADER_OBJ16 += $(call find_objs_names,cpp,16.elf,$(LOADER_SRC16),$(LOADER_TMP))

override LOADER_OBJ32 :=
override LOADER_OBJ32 += $(call find_objs_names,asm,.elf,$(LOADER_SRC32),$(LOADER_TMP))
override LOADER_OBJ32 += $(call find_objs_names,cpp,32.elf,$(LOADER_SRC32),$(LOADER_TMP))


override GNU_LD_RWX_SEGMENT_WARNING = \
$(shell (echo "SECTIONS{}" | \
	$(LINKER_FOR_TARGET) /dev/null -T /dev/stdin -nostartfiles \
	-Wl,--no-warn-rwx-segment 2>/dev/null -o /dev/zero) \
&& echo ,--no-warn-rwx-segment || echo)

override LOADER_CXX_ARGS := \
	-mgeneral-regs-only \
	-march=i486 \
	-Wno-return-type-c-linkage \
	-I$(LOADER_DIR)



$(LOADER_LAYOUT):
	mkdir -p $(LOADER_TMP)
	$(call replicate_directory_structure,$(LOADER_SRC16),$(LOADER_TMP))
	$(call replicate_directory_structure,$(LOADER_SRC32),$(LOADER_TMP))

$(LOADER_TMP)/%.asm.elf: $(LOADER_DIR)/src*/%.asm | $(LOADER_LAYOUT)
	$(NASMD) $< -f elf32 -o $@

$(LOADER_TMP)/%.cpp16.elf: $(LOADER_SRC16)/%.cpp | $(LOADER_LAYOUT)
	$(CXX16) $< -o $@ $(call get_osize,$(CXX16)) $(LOADER_CXX_ARGS)

$(LOADER_TMP)/%.cpp32.elf: $(LOADER_SRC32)/%.cpp | $(LOADER_LAYOUT)
	$(CXX32) $< -o $@ $(call get_osize,$(CXX32)) $(LOADER_CXX_ARGS)

$(LOADER_ELF16): $(LOADER_OBJ16) $(LOADER_DIR)/loader16.ld
	$(LINKER_FOR_TARGET) -r -o $@ $(LOADER_OBJ16) -T $(LOADER_DIR)/loader16.ld
	$(OBJCOPY_TARGET) -w -L \* $@ $@
	$(OBJCOPY_TARGET) --globalize-symbol=edata $@ $@
	$(OBJCOPY_TARGET) --globalize-symbol=kernel_listing_sector $@ $@
	$(OBJCOPY_TARGET) --globalize-symbol=pbr_disk $@ $@

$(LOADER_ELF32): $(LOADER_OBJ32)
	$(LINKER_FOR_TARGET) -Wl,-melf_i386 -r -o $@ $(LOADER_OBJ32)
	$(OBJCOPY_TARGET) -w -L \* $@ $@
	$(OBJCOPY_TARGET) --globalize-symbol=go_pm $@ $@

$(LOADER_ELF): $(LOADER_ELF16) $(LOADER_ELF32) $(LOADER_DIR)/loader.ld
	$(LINKER_FOR_TARGET) $(LOADER_ELF16) $(LOADER_ELF32) -o $(LOADER_ELF) \
		-T $(LOADER_DIR)/loader.ld -Wl$(GNU_LD_RWX_SEGMENT_WARNING),--build-id=none

$(LOADER_BIN): $(LOADER_ELF)
	$(OBJCOPY_TARGET) $(LOADER_ELF) $@ -O binary
	chmod a-x $@ $(LOADER_ELF) $(LOADER_BIN)
	ndisasm -o 0x600 $@ > $(LOADER_DISASM)

clean-sub-$(LOADER_DIR):
	rm -rf $(LOADER_TMP)
	rm -f $(LOADER_ELF) $(LOADER_BIN) $(LOADER_DISASM)
