ifeq (0,1)
	  This file is a part of StOS project - a small operating system
	  	made for learning purposes
	  Copyright (C) 2021 Egorov Stanislav, kodirovsshik@mail.ru, kodirovsshik@gmail.com

	  This program is free software: you can redistribute it and/or modify
	  it under the terms of the GNU General Public License as published by
	  the Free Software Foundation, either version 3 of the License, or
	  (at your option) any later version.

	  This program is distributed in the hope that it will be useful,
	  but WITHOUT ANY WARRANTY; without even the implied warranty of
	  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	  GNU General Public License for more details.

	  You should have received a copy of the GNU General Public License
	  along with this program.  If not, see <https://www.gnu.org/licenses/>.
endif


SHELL := /bin/bash

override LOCAL_PATH := $(shell pwd)

NOECHO := 1>/dev/null 2>&1


SYSTEM_DISK_FILE := system/disk
SYSTEM_DISK_SIZE := 8
SYSTEM_DISK_FILE_VB := system/vb_disk.vmdk



#MBR_SYSTEM_PARTITION_TYPE :=



QEMU32_MEMORY_MB := 4096
QEMU64_MEMORY_MB := 6144
QEMU32_CPU := base,+cmov
QEMU64_CPU := base,+cmov,+lm
override QEMU_COMMON_ARGS := -no-reboot -no-shutdown \
	-drive file=$(SYSTEM_DISK_FILE),format=raw \
	$(QEMU_ARGS)
override QEMU32_COMMON_ARGS := -m size=$(QEMU32_MEMORY_MB) -cpu $(QEMU32_CPU)
override QEMU64_COMMON_ARGS := -m size=$(QEMU64_MEMORY_MB) -cpu $(QEMU64_CPU)
QEMU_DEBUG_ARGS := -s -S

override QEMU32 := qemu-system-i386   $(QEMU32_COMMON_ARGS) $(QEMU_COMMON_ARGS)
override QEMU64 := qemu-system-x86_64 $(QEMU64_COMMON_ARGS) $(QEMU_COMMON_ARGS)


override _NASM_FLAGS :=
override _NASM = nasm $(NASM_FLAGS) $(_NASM_FLAGS) -Wall -Werror -Wno-label-redef-late -Wno-gnu-elf-extensions -gdwarf
override NASM = $(_NASM) -I$(<D)


ifeq ($(DEBUG),no)
override DBG := -Os -DNDEBUG
else
override DBG := -g -DDEBUG
endif

override _CC_ARGS := $(CC_ARGS) -std=c++20 -c -ffreestanding -mno-red-zone -Wall -Wextra -Werror -lgcc $(DBG)
override _CXX_ARGS := $(CXX_ARGS) $(_CC_ARGS) -fno-rtti -fno-exceptions

override CC_INCLUDE_BOOT := -I$(LOCAL_PATH)/src/boot/include

CC64 := x86_64-elf-gcc
CC32 := i686-elf-gcc

CXX64 := x86_64-elf-g++
CXX32 := i686-elf-g++

CC := gcc
CXX := g++

override _CC_HOST_ARGS := -Wall -Wextra -Werror -std=c++20
override _CC := $(CC) $(_CC_HOST_ARGS)
override _CXX := $(CXX) $(_CC_HOST_ARGS)

override _CC32 := $(CC32) $(_CC_ARGS)
override _CC64 := $(CC64) $(_CC_ARGS)

override _CXX32 := $(CXX32) $(_CXX_ARGS)
override _CXX64 := $(CXX64) $(_CXX_ARGS)

override LINK32 := $(CXX32) -nostdlib $(LINK_OPTIONS)
override LINK64 := $(CXX64) -nostdlib $(LINK_OPTIONS)

OBJCOPY32 := i686-elf-objcopy
OBJCOPY64 := x86_64-elf-objcopy

GDB := gdb
override _GDB_ARGS := -x stuff/gdb/gdb_defs
override GDB32 := $(GDB) $(_GDB_ARGS) $(GDB_ARGS) -x stuff/gdb/gdb_script32
override GDB64 := $(GDB) $(_GDB_ARGS) $(GDB_ARGS) -x stuff/gdb/gdb_script64



STOS_LOADER_REQS := src/link/bootloader.ld
STOS_LOADER_REQS += temp/boot/bootloader.asm.elf32
STOS_LOADER_REQS += temp/boot/cpuid.asm.elf32
STOS_LOADER_REQS += temp/boot/interrupt.asm.elf32
STOS_LOADER_REQS += temp/boot/string.asm.elf32
STOS_LOADER_REQS += temp/boot/aux.asm.elf32

STOS_LOADER_REQS += temp/boot/main.cpp.elf32
STOS_LOADER_REQS += temp/boot/memory.cpp.elf32
STOS_LOADER_REQS += temp/boot/bootloader.cpp.elf32
STOS_LOADER_REQS += temp/boot/init.cpp.elf32

STOS_LOADER_ENTRY_MBR_OBJ := temp/boot/entry_mbr.asm.elf32
STOS_LOADER_ENTRY_VBR_OBJ := temp/boot/entry_vbr.asm.elf32
STOS_LOADER_ENTRY_GPT_OBJ := temp/boot/entry_gpt.asm.elf32

STOS_LOADER_MBR_ELF := result/bootloader_mbr.elf
STOS_LOADER_VBR_ELF := result/bootloader_vbr.elf
STOS_LOADER_GPT_ELF := result/bootloader_gpt.elf

STOS_LOADER_MBR_RAW := result/bootloader_mbr.img
STOS_LOADER_VBR_RAW := result/bootloader_vbr.img
STOS_LOADER_GPT_RAW := result/bootloader_gpt.img



INIT_BUILD_DIRS := temp/init


PARTITIONER_MBR := stuff/partitioner/mbr
PARTITIONER_GPT := stuff/partitioner/gpt
BURNER := stuff/burner/a
MBR_CHECKER := stuff/mbr_detecter/a
GPT_CHECKER := stuff/mbr_detecter/a --gpt

CRC32_IMPL := src/crc/crc32.cpp





CREATE_OUTPUT_DIR = X="$@" ; mkdir -p $${X%/*}





.PHONY: all clean clean_wipe rebuild host_boot host_boot_debug
.PHONY: debug32_qemu debug64_qemu system_burn_mbr system_burn_gpt
.PHONY: system_create_mbr system_create_gpt system_wipe_disk_layout
.PHONY: _system_create_mbr _system_create_gpt 
.PHONY: run32 run32v run64 run64v todo build_init auxillary
.PHONY: stos_loader_mbr stos_loader_vbr stos_loader_gpt



all: stos_loader_mbr stos_loader_vbr stos_loader_gpt



rebuild: clean
	$(MAKE)



todo:
	grep -nRi todo ./src ./stuff



stos_loader_mbr: $(STOS_LOADER_MBR_RAW)

stos_loader_vbr: $(STOS_LOADER_VBR_RAW)

stos_loader_gpt: $(STOS_LOADER_GPT_RAW)



QEMU_HOST_ARGS := -m 3072 -no-reboot -no-shutdown \
	-drive file=$(SYSTEM_DISK_FILE),format=raw \
	-drive file=/dev/sda,format=raw \
	-drive file=/dev/sdb,format=raw \

host_boot_debug:
	qemu-system-i386 $(QEMU_HOST_ARGS) -S -s &
	$(GDB32) || true

host_boot:
	qemu-system-x86_64 $(QEMU_HOST_ARGS) --enable-kvm --boot menu=on




$(STOS_LOADER_MBR_RAW): $(STOS_LOADER_MBR_ELF)
$(STOS_LOADER_MBR_ELF): $(STOS_LOADER_REQS) $(STOS_LOADER_ENTRY_MBR_OBJ)
$(STOS_LOADER_ENTRY_MBR_OBJ): NASM_FLAGS += -D_MBR_MODE

$(STOS_LOADER_VBR_RAW): $(STOS_LOADER_VBR_ELF)
$(STOS_LOADER_VBR_ELF): $(STOS_LOADER_REQS) $(STOS_LOADER_ENTRY_VBR_OBJ)
$(STOS_LOADER_ENTRY_VBR_OBJ): NASM_FLAGS += -D_VBR_MODE

$(STOS_LOADER_GPT_RAW): $(STOS_LOADER_GPT_ELF)
$(STOS_LOADER_GPT_ELF): $(STOS_LOADER_REQS) $(STOS_LOADER_ENTRY_GPT_OBJ)



$(STOS_LOADER_MBR_ELF) $(STOS_LOADER_GPT_ELF) $(STOS_LOADER_VBR_ELF):
	$(LINK32) -T $^ -o $@

$(STOS_LOADER_MBR_RAW) $(STOS_LOADER_GPT_RAW) $(STOS_LOADER_VBR_RAW):
	$(OBJCOPY32) -R .canary -O binary $< $@
	ndisasm -o 0x400 $@ > $@.disasm
	xxd -o 0x400 $@ > $@.hexdump



clean:
	rm -rf temp result/*



clean_wipe:
	rm -rf system temp result
	rm -f $(PARTITIONER_MBR) $(PARTITIONER_GPT) $(BURNER) $(MBR_CHECKER)



auxillary: $(PARTITIONER_MBR) $(PARTITIONER_GPT) $(BURNER) $(MBR_CHECKER)



$(INIT_BUILD_DIRS):
	tree -dfi --noreport src | xargs -i mkdir -p "temp/{}"
	mv temp/src/* temp
	rm -r temp/src
	mkdir -p result
	touch $@



temp/boot/%.asm.elf32: src/boot/%.asm $(INIT_BUILD_DIRS)
	$(NASM) -f elf32 $< -o $@

temp/boot/%.asm.elf64: src/boot/%.asm $(INIT_BUILD_DIRS)
	$(NASM) -f elf64 $< -o $@

temp/boot/shared/disk.cpp.elf32: src/boot/shared/disk.cpp $(INIT_BUILD_DIRS)
	$(_CXX32) $< -o $@ $(CC_INCLUDE_BOOT) -include src/boot/shared/include/aux.h

temp/boot/%.c.elf32: src/boot/%.c $(INIT_BUILD_DIRS)
	$(_CC32) $< -o $@ $(CC_INCLUDE_BOOT)

temp/boot/%.cpp.elf32: src/boot/%.cpp $(INIT_BUILD_DIRS)
	$(_CXX32) $< -o $@ $(CC_INCLUDE_BOOT)

temp/boot/%.c.elf64: src/boot/%.c $(INIT_BUILD_DIRS)
	$(_CC64) $< -o $@ $(CC_INCLUDE_BOOT)

temp/boot/%.cpp.elf64: src/boot/%.cpp $(INIT_BUILD_DIRS)
	$(_CXX64) $< -o $@ $(CC_INCLUDE_BOOT)



debug32_qemu:
	$(QEMU32) $(QEMU_DEBUG_ARGS) &
	$(GDB32) || true



debug64_qemu:
	$(QEMU64) $(QEMU_DEBUG_ARGS) &
	$(GDB64) || true



VNC_DISPLAY := 0

run32:
	$(QEMU32)

run32v:
	$(QEMU32) -display vnc=:$(VNC_DISPLAY)

run64:
	$(QEMU64)

run64v:
	$(QEMU64) -display vnc=:$(VNC_DISPLAY)



system_wipe:
	rm -rf $(SYSTEM_DISK_FILE) $(SYSTEM_DISK_FILE_VB)
	mkdir -p system

	dd if=/dev/zero of=$(SYSTEM_DISK_FILE) bs=1M count=$(SYSTEM_DISK_SIZE)
	VBoxManage internalcommands createrawvmdk -filename $(SYSTEM_DISK_FILE_VB) -rawdisk "$(LOCAL_PATH)/$(SYSTEM_DISK_FILE)" || true



system_wipe_disk_layout:
	dd if=/dev/zero bs=1024 count=1 of=$(SYSTEM_DISK_FILE) conv=notrunc
	dd if=/dev/zero bs=512  count=1 of=$(SYSTEM_DISK_FILE) conv=notrunc seek=$$(($(SYSTEM_DISK_SIZE) * 1048576 / 512 - 1))



define remake_partition_table
	stat $(NOECHO) $(SYSTEM_DISK_FILE) ;\
	if [ "$$?" = "1" ] ;\
	then \
		$(MAKE) system_wipe ;\
	else \
		$(MAKE) system_wipe_disk_layout ;\
	fi ;
	$(MAKE) $(1)
endef

system_create_mbr:
	$(call remake_partition_table,_system_create_mbr)

system_create_gpt:
	$(call remake_partition_table,_system_create_gpt)


_system_create_mbr: $(PARTITIONER_MBR)
	$(PARTITIONER_MBR) $(SYSTEM_DISK_FILE) 1 0 0 0 0
	$(PARTITIONER_MBR) $(SYSTEM_DISK_FILE) 2 0 0 0 0
	$(PARTITIONER_MBR) $(SYSTEM_DISK_FILE) 3 0 0 0 0
	$(PARTITIONER_MBR) $(SYSTEM_DISK_FILE) 4 0 0 0 0


_system_create_gpt: $(PARTITIONER_GPT)
	$(PARTITIONER_GPT) $(SYSTEM_DISK_FILE) table 128
	$(PARTITIONER_GPT) $(SYSTEM_DISK_FILE) create 0 36 291
	$(PARTITIONER_GPT) $(SYSTEM_DISK_FILE) set 0 type_str "StOS bootloader "



$(PARTITIONER_MBR): $(PARTITIONER_MBR).cpp
	$(_CXX) $^ -o $@ -g $(CC_INCLUDE_BOOT)
$(PARTITIONER_GPT): $(PARTITIONER_GPT).cpp $(CRC32_IMPL)
	$(_CXX) $^ -o $@ -g $(CC_INCLUDE_BOOT)
$(BURNER): $(BURNER).cpp
	$(_CXX) $^ -o $@ -g $(CC_INCLUDE_BOOT) src/boot/disk.cpp -include string.h
$(MBR_CHECKER): $(MBR_CHECKER).cpp
	$(_CXX) $^ -o $@ -g



define action_burn
	stat $(NOECHO) $(SYSTEM_DISK_FILE) || $(MAKE) system_wipe

	$(2) $(SYSTEM_DISK_FILE) || $(MAKE) $(3)

	$(BURNER) mbr $(SYSTEM_DISK_FILE) $(1)
endef



system_burn_mbr: $(STOS_LOADER_MBR_RAW) $(VBR_RAW) auxillary
	$(call action_burn,$<,$(MBR_CHECKER),_system_create_mbr)

system_burn_gpt: $(STOS_LOADER_GPT_RAW) $(VBR_RAW) auxillary
	$(call action_burn,$<,$(GPT_CHECKER),_system_create_gpt)
