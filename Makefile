
#Clang (and apparently GCC starting with 13.0) have aggressive size optimization option -Oz
ifeq ($(DEBUG),true)
define get_osize
$(shell echo)
endef
else
define get_osize
$(shell ($(1) -xc /dev/null -c -Oz 2>/dev/null -o /dev/zero) \
	&& echo -Oz || echo -Os)
endef
endif


#Invoke as follows: find_objs_names (src excention) (appendix) (src dir) (tmp dir)
#Finds all files with extention (src excention) in (src dir)
#Returns list of files of the form (tmp dir)/filename(appendix)
define find_objs_names
$(patsubst \
$(3)/%.$(1),\
$(4)/%.$(1)$(2),\
$(shell find $(3) -type f -name \*.$(1))\
)
endef



#Invoke as follows: replicate_directory_structure SRC DST
#Replicates structure of SRC inside DST
define replicate_directory_structure
	cd $(1) ;\
	find . -type d -exec mkdir -p -- "$(shell pwd)/$(2)/{}" \;
endef
#why does find require a semicolon at the end wtf



override SILENT := >/dev/null
override ESILENT := 2>/dev/null
override DETACHED := $(SILENT) $(ESILENT) & true

override _CXX_ARGS += -Wno-unused-function
override _NASM_ARGS += -Wall -Werror -Wno-unknown-warning -Ox

#Customization points (compilers for specific targets)
CXX_FOR_TARGET := x86_64-pc-elf-g++
CXX64_FOR_TARGET := $(CXX_FOR_TARGET) -m64 -mcmodel=large
CXX32_FOR_TARGET := $(CXX_FOR_TARGET) -m32
CXX16_FOR_TARGET := $(CXX_FOR_TARGET) -m16

override CXX_OTIME :=

ifeq ($(DEBUG),true)
override _CXX_ARGS += -Wall -Wextra -Werror -g -O0 -D_DEBUG
override _NASM_ARGS += -D_DEBUG
else
override _CXX_ARGS += -DNDEBUG
override _NASM_ARGS += -DNDEBUG
override CXX_OTIME += -Ofast
endif

override _CXX_FOR_TARGET_ARGS := -c $(_CXX_ARGS) -ffreestanding -fno-exceptions -fno-rtti
override _LINKER_FOR_TARGET_ARGS := -nostdlib

#Customization points (args for different compilers)
override export CXX64 := $(CXX64_FOR_TARGET) $(CXX_FOR_TARGET_ARGS) $(CXX64_FOR_TARGET_ARGS) $(_CXX_FOR_TARGET_ARGS)
override export CXX32 := $(CXX32_FOR_TARGET) $(CXX_FOR_TARGET_ARGS) $(CXX32_FOR_TARGET_ARGS) $(_CXX_FOR_TARGET_ARGS)
override export CXX16 := $(CXX16_FOR_TARGET) $(CXX_FOR_TARGET_ARGS) $(CXX16_FOR_TARGET_ARGS) $(_CXX_FOR_TARGET_ARGS)

#Customization point (args for linker)
override export LINKER_FOR_TARGET := $(CXX_FOR_TARGET) $(LINKER_FOR_TARGET_ARGS) $(_LINKER_FOR_TARGET_ARGS)

override export _CXX := $(CXX) $(CXX_OTIME) $(_CXX_ARGS)

#Customization points (custom NASM and args)
NASM := nasm
override NASM := $(NASM) $(NASM_ARGS) $(_NASM_ARGS)
ifeq ($(DEBUG),true)
override NASMD := $(NASM) -gdwarf
else
override NASMD := $(NASM)
endif

#Customization point (custom objcopy)
export OBJCOPY_TARGET := objcopy


#Customization points (vm settings)
VM_DIR := vm
VM_DISK := $(VM_DIR)/disk.bin
VM_DISK_SIZE_MiB := 2
VM_MEMORY_MiB := 32

override VM_MNT := $(VM_DIR)/mnt

override _QEMU_ARGS := $(QEMU_ARGS) -m $(VM_MEMORY_MiB) -drive file="$(VM_DISK)",format=raw

#Customization points
QEMU32 := qemu-system-i386
QEMU32_CPU := 486
QEMU64 := qemu-system-x86_64
QEMU64_CPU := Opteron_G1-v1

override RUN_QEMU32 := $(QEMU32) $(_QEMU_ARGS) $(QEMU32_ARGS) -cpu $(QEMU32_CPU)
override RUN_QEMU64 := $(QEMU64) $(_QEMU_ARGS) $(QEMU64_ARGS) -cpu $(QEMU64_CPU)

#Customization point
EMULATOR_LOG_FILE := emu.log

override export MiB := 1048576



.PHONY: all
all: binaries
#target 'binaries' will have all the necessary dependencies known after all includes


override UTILS :=
override UTILS_DIRS :=



#Customization point (output binary name)
VM_PREPARATOR_BIN_NAME := vm_preparator.elf
override VM_PREPARATOR_DIR := vm_preparator
override VM_PREPARATOR_BIN := $(VM_PREPARATOR_DIR)/$(VM_PREPARATOR_BIN_NAME)
override VM_PREPARATOR := $(VM_PREPARATOR_BIN)
override UTILS_DIRS += $(VM_PREPARATOR_DIR)
override UTILS += $(VM_PREPARATOR_BIN)
include $(VM_PREPARATOR_DIR)/Makefile




override BINARIES :=
override BINARIES_DIRS :=


#Customization point (output binary name)
PBR_BIN_NAME := pbr.bin
override PBR_DIR := pbr
override PBR_BIN := $(PBR_DIR)/$(PBR_BIN_NAME)
override BINARIES_DIRS += $(PBR_DIR)
override BINARIES += $(PBR_BIN)
include $(PBR_DIR)/Makefile


#Customization point (output binary name)
MBR_BIN_NAME := mbr.bin
override MBR_DIR := mbr
override MBR_BIN := $(MBR_DIR)/$(MBR_BIN_NAME)
override BINARIES_DIRS += $(MBR_DIR)
override BINARIES += $(MBR_BIN)
include $(MBR_DIR)/Makefile


#Customization point (output binary name)
LOADER_BIN_NAME := loader.bin
override LOADER_DIR := loader
override LOADER_BIN := $(LOADER_DIR)/$(LOADER_BIN_NAME)
override BINARIES_DIRS += $(LOADER_DIR)
override BINARIES += $(LOADER_BIN)
include $(LOADER_DIR)/Makefile

override LOADER_SIZE = $(shell stat -c "%s" $(LOADER_BIN))
override LOADER_SECTORS = $(shell expr \( $(LOADER_SIZE) + 511 \) / 512 )


#Customization point (output binary name)
KERNEL_BIN_NAME := kernel.bin
override KERNEL_DIR := kernel
override KERNEL_BIN := $(KERNEL_DIR)/$(KERNEL_BIN_NAME)
override BINARIES_DIRS += $(KERNEL_DIR)
override BINARIES += $(KERNEL_BIN)
include $(KERNEL_DIR)/Makefile



override SUBDIRS := $(UTILS_DIRS) $(BINARIES_DIRS) 





.PHONY: binaries utils everything

binaries: $(BINARIES)
utils: $(UTILS)
everything: binaries utils



.PHONY: clean clean-binaries clean-utils wipe

clean-binaries: $(patsubst %,clean-sub-%,$(BINARIES_DIRS))

clean-utils: $(patsubst %,clean-sub-%,$(UTILS_DIRS))

clean: clean-binaries clean-utils

wipe: clean vm-clean




.PHONY: vm-create vm-clean vm-recreate vm-burn qemu-run vm-debug16



$(VM_DISK): $(MBR_BIN)
	mkdir -p $(VM_DIR)
	dd if=/dev/zero of=$(VM_DISK) bs=1M count=$(VM_DISK_SIZE_MiB)
	sfdisk $(VM_DISK) >/dev/null <sfdisk_script
	$(call write_boot_record,$(MBR_BIN),"$(VM_DISK)",0)


vm-create: $(VM_DISK)

vm-clean:
	rm -rf $(VM_DIR)

vm-reburn: vm-clean
	$(MAKE) vm-burn

vm-burn: $(VM_DISK) $(BINARIES) $(UTILS)
	sudo $(VM_PREPARATOR) $(VM_DISK) $(VM_MNT) $(KERNEL_BIN) $(PBR_BIN) $(LOADER_BIN)
	$(call write_boot_record,$(PBR_BIN),"$(VM_DISK)",$(MiB))
	sfdisk -A $(VM_DISK) 1 >/dev/null



define write_boot_record
	[ $$(stat -c "%s" "$(1)") -eq 512 ]
	dd if=$(1) of=$(2) bs=1 conv=notrunc seek=$(3) count=3 $(ESILENT)
	dd if=$(1) of=$(2) bs=1 conv=notrunc seek=$$(($(3)+90)) skip=90 count=350 $(ESILENT)
	dd if=$(1) of=$(2) bs=1 conv=notrunc seek=$$(($(3)+510)) skip=510 count=2 $(ESILENT)
endef

define write_image
	dd if=$(1) of=$(2) bs=512 conv=notrunc seek=$(3) oflag=seek_bytes $(ESILENT)
endef




qemu-run32: vm-burn
	$(RUN_QEMU32)
qemu-run64: vm-burn
	$(RUN_QEMU64)
qemu-run: vm-burn
	$(RUN_QEMU64) -cpu host --enable-kvm

define qemu_debug
	$(call vm_debug,$(1) -S -s,$(2))
endef
define vm_debug
	$(MAKE) vm-burn
	$(1) >$(EMULATOR_LOG_FILE) 2>&1 & true ;\
	gdb -x gdb/defs.gdb \
		-x gdb/init.gdb \
		-x $(2) \
		$(GDB_ARGS) || true;\
	kill -9 $$! || true
endef

qemu-debug16:
	$(call qemu_debug,$(RUN_QEMU32),gdb/init16.gdb)
qemu-debug32:
	$(call qemu_debug,$(RUN_QEMU32),gdb/init32.gdb)
qemu-debug64:
	$(call qemu_debug,$(RUN_QEMU64),gdb/init64.gdb)


define bochs_debug
	rm -f $(VM_DISK).lock
	$(call vm_debug,bochs -qf $(1),$(2))
endef

bochs-run32: vm-burn
	rm -f $(VM_DISK).lock
	bochs -qf bochs/conf32

bochs-run64: vm-burn
	rm -f $(VM_DISK).lock
	bochs -qf bochs/conf64

bochs-debug64:
	$(call bochs_debug,bochs/conf64d,gdb/init64.gdb)



#burns $(VM_DISK) to my USB stick $(dev)
#for ease of testing on real hardware
dev := /dev/sdd
_:
#	It would suck to wipe someone's MBR by accident
	[ x$$(whoami) = xkodirovsshik ]
	sudo fdisk -l $(dev) | grep -i myusb $(SILENT)
	$(MAKE) vm-burn
	sudo dd if=$(VM_DISK) of=$(dev) bs=1M count=2

print_serial_port_dump:
	dd if=$(VM_DISK) bs=512 skip=1920 count=128 $(ESILENT) | cat
