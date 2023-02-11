
override KERNEL_ELF = $(shell echo $(KERNEL_BIN) | sed s/.bin/.elf/g)

override KERNEL_SRC := $(KERNEL_DIR)/src
override KERNEL_TMP := $(KERNEL_DIR)/tmp

override KERNEL_OBJS :=
override KERNEL_OBJS += $(call find_objs_names,asm,.elf,$(KERNEL_SRC),$(KERNEL_TMP))
override KERNEL_OBJS += $(call find_objs_names,cpp,64.elf,$(KERNEL_SRC),$(KERNEL_TMP))

override KERNEL_CXX_ARGS := $(CXX_OTIME) -std=c++20 -masm=intel -mabi=sysv -mno-red-zone -march=opteron

override KERNEL_HEADERS := $(shell find $(KERNEL_DIR)/include -type f -name \*.h\*)

override KERNEL_LAYOUT := $(KERNEL_TMP)


$(KERNEL_LAYOUT):
	mkdir -p $(KERNEL_TMP)
	$(call replicate_directory_structure,$(KERNEL_SRC),$(KERNEL_TMP))

$(KERNEL_TMP)/%.asm.elf: $(KERNEL_SRC)/%.asm | $(KERNEL_LAYOUT)
	$(NASMD) $< -f elf64 -o $@

$(KERNEL_TMP)/%.cpp64.elf: $(KERNEL_SRC)/%.cpp $(KERNEL_HEADERS) | $(KERNEL_LAYOUT)
	$(CXX64) $< -o $@ $(KERNEL_CXX_ARGS) -I$(KERNEL_DIR)/include

$(KERNEL_BIN): $(KERNEL_DIR)/kernel.ld $(KERNEL_OBJS)
	$(LINKER_FOR_TARGET) $(KERNEL_OBJS) -o $(KERNEL_ELF) -T $(KERNEL_DIR)/kernel.ld
	$(OBJCOPY_TARGET) -O binary $(KERNEL_ELF) $@
	chmod a-x $@ $(KERNEL_ELF) $(KERNEL_BIN)

clean-sub-$(KERNEL_DIR):
	rm -rf $(KERNEL_TMP)
	rm -f $(KERNEL_ELF) $(KERNEL_BIN)

make-sub-$(KERNEL_DIR): $(KERNEL_BIN)
