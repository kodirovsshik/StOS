
include Makefile.inc

override SILENT := >/dev/null
override ERR_SILENT := 2>/dev/null
override DETACHED := $(SILENT) $(ERR_SILENT) & true

override _CXX_ARGS += -Wno-unused-function
override _NASM_ARGS += -Wall -Werror -Ox -Wno-unknown-warning

#Customization points (compilers for specific targets)
CXX_FOR_TARGET := x86_64-pc-elf-g++
CXX64_FOR_TARGET := $(CXX_FOR_TARGET) -m64
CXX32_FOR_TARGET := $(CXX_FOR_TARGET) -m32
CXX16_FOR_TARGET := $(CXX_FOR_TARGET) -m16

override CXX_OTIME :=

ifeq ($(DEBUG),true)
override _CXX_ARGS += -Wall -Wextra -Werror -g -Og
override _NASM_ARGS += -g
else
override CXX_OTIME += -Ofast
endif

export CXX_OTIME

override _CXX_FOR_TARGET_ARGS := -c $(_CXX_ARGS) -ffreestanding -fno-exceptions -fno-rtti
override _LINKER_FOR_TARGET_ARGS := -nostdlib

#Customization points (args for different compilers)
override export CXX64 := $(CXX64_FOR_TARGET) $(CXX_FOR_TARGET_ARGS) $(CXX64_FOR_TARGET_ARGS) $(_CXX_FOR_TARGET_ARGS) -mno-red-zone
override export CXX32 := $(CXX32_FOR_TARGET) $(CXX_FOR_TARGET_ARGS) $(CXX32_FOR_TARGET_ARGS) $(_CXX_FOR_TARGET_ARGS)
override export CXX16 := $(CXX16_FOR_TARGET) $(CXX_FOR_TARGET_ARGS) $(CXX16_FOR_TARGET_ARGS) $(_CXX_FOR_TARGET_ARGS)

#Customization point (args for linker)
override export LINKER_FOR_TARGET := $(CXX_FOR_TARGET) $(LINKER_FOR_TARGET_ARGS) $(_LINKER_FOR_TARGET_ARGS)

override export _CXX := $(CXX) $(CXX_OTIME) $(_CXX_ARGS)

#Customization points (custom NASM and args)
NASM := nasm
override NASM := $(NASM) $(NASM_ARGS) $(_NASM_ARGS)
export NASM

#Customization point (custom objcopy)
export OBJCOPY_TARGET := objcopy

override LAYOUT := result/layout

override UTILS_SUBDIRS := binecho
override SRC_SUBDIRS := mbr pbr loader
override SUBDIRS := $(UTILS_SUBDIRS) $(SRC_SUBDIRS) 

#Customization points (vm settings)
VM_DIR := vm
VM_DISK := $(VM_DIR)/disk.bin
VM_DISK_SIZE_MiB := 8
VM_MEMORY_MiB := 64

override _QEMU_ARGS := \
	$(QEMU_ARGS) \
	-m $(VM_MEMORY_MiB) \
	-drive file="$(VM_DISK)",format=raw \

#Customization points
QEMU32 := qemu-system-i386
QEMU32_CPU := 486
QEMU64 := qemu-system-x86_64
QEMU64_CPU := 486,+lm,+pae

override RUN_QEMU32 := $(QEMU32) $(_QEMU_ARGS) $(QEMU32_ARGS) -cpu $(QEMU32_CPU)
override RUN_QEMU64 := $(QEMU64) $(_QEMU_ARGS) $(QEMU64_ARGS) -cpu $(QEMU64_CPU)

override export MiB := 1048576




.PHONY: all $(SUBDIRS) rebuild clean reset wipe
.PHONY: vm-create vm-clean vm-recreate vm-burn vm-run32 vm-debug32


all: $(SUBDIRS)

$(SRC_SUBDIRS): $(UTILS_SUBDIRS)

rebuild: clean
	$(MAKE)

clean:
	rm -rf result
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done

reset: clean vm-clean
wipe: reset

$(SUBDIRS): $(LAYOUT)
	$(MAKE) -C $@

$(LAYOUT):
	mkdir -p result result/utils
	touch $@



vm-create: $(VM_DISK)

vm-clean:
	rm -rf $(VM_DIR)

vm-recreate: vm-clean
	$(MAKE) vm-create


$(VM_DISK):
	mkdir -p $(VM_DIR)
	dd if=/dev/zero of=$@ bs=1M count=$(VM_DISK_SIZE_MiB)
	bash -c "echo -e 1M,\\\\nwrite | sfdisk $@" >/dev/null
# ^^^ I hate myself for writing this ^^^
#but it keeps -e in the output for some reason if i don't wrap it with bash -c
	sfdisk -A $@ 1

define write_boot_record
	[ $$(stat -c "%s" "$(1)") -eq 512 ]
	dd if=$(1) of=$(2) bs=1 conv=notrunc seek=$(3) count=3 $(ERR_SILENT)
	dd if=$(1) of=$(2) bs=1 conv=notrunc seek=$$(($(3)+90)) skip=90 count=350 $(ERR_SILENT)
	dd if=$(1) of=$(2) bs=1 conv=notrunc seek=$$(($(3)+510)) skip=510 count=2 $(ERR_SILENT)
endef

define write_image
	dd if=$(1) of=$(2) bs=512 conv=notrunc seek=$(3) oflag=seek_bytes
endef

vm-burn: $(VM_DISK) all
	set -e ;\
	LOADER_SIZE=$$(stat -c "%s" result/loader.bin) ;\
	LOADER_SECTORS=$$((($$LOADER_SIZE + 511) / 512)) ;\
	result/binecho $$LOADER_SECTORS | (dd of=result/pbr.bin bs=1 count=2 seek=92 conv=notrunc 2>/dev/null)
	result/binecho 2049 | (dd of=result/pbr.bin bs=1 count=8 seek=98 conv=notrunc 2>/dev/null)
	$(call write_boot_record,result/mbr.bin,"$(VM_DISK)",0)
	$(call write_boot_record,result/pbr.bin,"$(VM_DISK)",$(MiB))
	$(call write_image,result/loader.bin,"$(VM_DISK)",$$(($(MiB)+512)))

vm-run32: vm-burn
	$(RUN_QEMU32)
vm-run64: vm-burn
	$(RUN_QEMU64)
vm-run-kvm: vm-burn
	$(RUN_QEMU64) -cpu host --enable-kvm

define vm_debug
	$(1) -S -s $(DETACHED) ;\
	gdb -x gdb/defs.gdb \
		-x gdb/init.gdb \
		-x $(2) \
		$(GDB_ARGS) ;\
	kill -9 $$! || true
endef

vm-debug16: vm-burn
	$(call vm_debug,$(RUN_QEMU32),gdb/init16.gdb)
#vm-debug32: vm-burn
#	$(call vm_debug,$(RUN_QEMU32),gdb/init32.gdb)
#vm-debug64: vm-burn
#	$(call vm_debug,$(RUN_QEMU64),gdb/init64.gdb)

#burns $(VM_DISK) to my USB stick $(dev)
#for ease of testing on real hardware
dev := /dev/sdd
_:
#	It would suck to wipe someone's MBR by accident
	[ x$$(whoami) = xkodirovsshik ]
	sudo fdisk -l $(dev) | grep -i myusb >/dev/null
	$(MAKE) vm-burn
	sudo dd if=$(VM_DISK) of=$(dev) bs=1M count=2

print_serial_port_dump:
	dd if=$(VM_DISK) bs=512 skip=1920 count=128 2>/dev/null | cat
