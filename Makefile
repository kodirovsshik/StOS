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


override SHELL := /bin/bash

override LOCAL_PATH := $(shell pwd)


SYSTEM_DRIVE1_FILE := system/drive
SYSTEM_DRIVE1_SIZE := 10
SYSTEM_DRIVE1_FILE_VB := system/vb_drive.vmdk

SYSTEM_DRIVE2_FILE := system/drive2
SYSTEM_DRIVE2_SIZE := 10
SYSTEM_DRIVE2_FILE_VB := system/vb_drive2.vmdk

SYSTEM_DRIVES :=
SYSTEM_DRIVES += $(SYSTEM_DRIVE1_FILE)
SYSTEM_DRIVES += $(SYSTEM_DRIVE1_FILE_VB)
SYSTEM_DRIVES += $(SYSTEM_DRIVE2_FILE)
SYSTEM_DRIVES += $(SYSTEM_DRIVE2_FILE_VB)


MBR_SYSTEM_PARTITION_TYPE := 0x83


QEMU32_MEMORY_MB := 4096
QEMU64_MEMORY_MB := 6144
QEMU32_CPU := base,+cmov
QEMU64_CPU := qemu64
override QEMU_COMMON_ARGS := -no-reboot -no-shutdown \
	-drive file=$(SYSTEM_DRIVE1_FILE),format=raw \
	-drive file=$(SYSTEM_DRIVE2_FILE),format=raw $(QEMU_ARGS)
override QEMU32_COMMON_ARGS := -m size=$(QEMU32_MEMORY_MB) -cpu $(QEMU32_CPU)
override QEMU64_COMMON_ARGS := -m size=$(QEMU64_MEMORY_MB) -cpu $(QEMU64_CPU)
QEMU_TEMPDBG_ARGS := -s -S

override QEMU32 := qemu-system-i386   $(QEMU32_COMMON_ARGS) $(QEMU_COMMON_ARGS)
override QEMU64 := qemu-system-x86_64 $(QEMU64_COMMON_ARGS) $(QEMU_COMMON_ARGS)


override _NASM := nasm -Wall -Wother -Werror -Wno-label-redef-late -Wno-gnu-elf-extensions -gdwarf
override NASM = $(_NASM) -I$(<D)


ifeq ($(DEBUG),no)
override DBG := -Os
else
override DBG := -g
endif

override _CC_ARGS := $(CC_ARGS) -std=c++20 -c -ffreestanding -mno-red-zone -Wall -Wextra -Werror $(DBG)
override _CXX_ARGS := $(CXX_ARGS) $(_CC_ARGS) -fno-rtti -fno-exceptions -std=c++20

override CC_INCLUDE_BOOT := -I$(LOCAL_PATH)/src/boot/shared/include

CC64 := x86_64-elf-gcc
CC32 := i686-elf-gcc
CC16 := $(CC32) -m16

CXX64 := x86_64-elf-g++
CXX32 := i686-elf-g++
CXX16 := $(CXX32) -m16

CC := gcc
CXX := g++

override _CC := $(CC)
override _CXX := $(CXX) -std=c++20

override _CC16 := $(CC16) $(_CC_ARGS)
override _CC32 := $(CC32) $(_CC_ARGS)
override _CC64 := $(CC64) $(_CC_ARGS)

override _CXX16 := $(CXX16) $(_CXX_ARGS)
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



STOS_LOADER_MBR_OBJS :=

STOS_LOADER_MBR_OBJS += temp/boot/mbr/main.cpp.elf16
STOS_LOADER_MBR_OBJS += temp/boot/mbr/bootloader.cpp.elf16
STOS_LOADER_MBR_OBJS += temp/boot/mbr/memory.cpp.elf16

STOS_LOADER_MBR_OBJS += temp/boot/mbr/entry.asm.elf32
STOS_LOADER_MBR_OBJS += temp/boot/mbr/bootloader.asm.elf32

STOS_LOADER_MBR_OBJS += temp/boot/shared/io.cpp.elf16
STOS_LOADER_MBR_OBJS += temp/boot/shared/disk.cpp.elf16

STOS_LOADER_MBR_OBJS += temp/boot/shared/interrupt.asm.elf32
STOS_LOADER_MBR_OBJS += temp/boot/shared/io.asm.elf32
STOS_LOADER_MBR_OBJS += temp/boot/shared/aux.asm.elf32

STOS_LOADER_MBR_ELF := result/bootloader_mbr.elf
STOS_LOADER_MBR_RAW := result/bootloader_mbr.img



VBR_OBJS :=

VBR_OBJS += temp/boot/vbr/entry.asm.elf32
VBR_OBJS += temp/boot/vbr/core.asm.elf32

VBR_OBJS += temp/boot/vbr/main.cpp.elf16
VBR_OBJS += temp/boot/vbr/rh_boot.cpp.elf16
VBR_OBJS += temp/boot/vbr/rh_get_boot_options.cpp.elf16
VBR_OBJS += temp/boot/vbr/native_boot.cpp.elf16

VBR_OBJS += temp/boot/shared/disk.cpp.elf16
VBR_OBJS += temp/boot/shared/io.cpp.elf16

VBR_OBJS += temp/boot/shared/io.asm.elf32
VBR_OBJS += temp/boot/shared/aux.asm.elf32
VBR_OBJS += temp/boot/shared/interrupt.asm.elf32

VBR_ELF := result/vbr.elf
VBR_RAW := result/vbr.img


INIT_BUILD_DIRS := temp/init


PARTITIONER_MBR := stuff/partitioner/mbr
BURNER := stuff/burner/a





CREATE_OUTPUT_DIR = X="$@" ; mkdir -p $${X%/*}





.PHONY: all clean bootloader loader rebuild host_boot host_boot_debug
.PHONY: debug32_qemu debug64_qemu system_wipe system_burn_mbr
.PHONY: run32 run32v run64 run64v todo build_init



all: stos_loader_mbr vbr



rebuild: clean all



todo:
	grep -nRi todo ./src ./stuff



stos_loader_mbr: $(STOS_LOADER_MBR_RAW)



vbr: $(VBR_RAW)



QEMU_HOST_ARGS := -m 3072 -no-reboot -no-shutdown \
	-drive file=$(SYSTEM_DRIVE1_FILE),format=raw,index=0 \
	-drive file=$(SYSTEM_DRIVE2_FILE),format=raw,index=1 \
	-drive file=/dev/sda,format=raw,index=2 \
	-drive file=/dev/sdb,format=raw,index=3 \

host_boot_debug:
	qemu-system-i386 $(QEMU_HOST_ARGS) -S -s &
	$(GDB32) || true

host_boot:
	qemu-system-x86_64 $(QEMU_HOST_ARGS) --enable-kvm --boot menu=on



$(STOS_LOADER_MBR_RAW): $(STOS_LOADER_MBR_OBJS)
	$(LINK32) -T src/link/bootmgr.ld $^ -o $(STOS_LOADER_MBR_ELF)
	$(OBJCOPY32) -O binary $(STOS_LOADER_MBR_ELF) $@
	ndisasm -o 0x400 $@ > $@.disasm
	xxd $@ > $@.disasm

$(VBR_RAW): $(VBR_OBJS)
	$(LINK32) -T src/link/vbr.ld $^ -o $(VBR_ELF)
	$(OBJCOPY32) -O binary $(VBR_ELF) $@
	ndisasm -o 0x7C00 $@ > $@.disasm
	xxd $@ > $@.disasm



clean:
	rm -rf temp/* result/*
	mkdir -p temp/boot/mbr



clean_wipe:
	rm -rf system temp result
	rm -f $(PARTITIONER_MBR) $(BURNER)



$(INIT_BUILD_DIRS):
	tree -dfi --noreport src | xargs -i mkdir -p "temp/{}"
	mv temp/src/* temp
	rm -r temp/src
	mkdir result
	touch $@



temp/%.asm.elf32: src/%.asm $(INIT_BUILD_DIRS)
	$(NASM) -f elf32 $< -o $@

temp/%.asm.elf64: src/%.asm $(INIT_BUILD_DIRS)
	$(NASM) -f elf64 $< -o $@


temp/boot/%.c.elf16: src/boot/%.c $(INIT_BUILD_DIRS)
	$(_CC16) $< -o $@ $(CC_INCLUDE_BOOT)

temp/boot/shared/disk.cpp.elf16: src/boot/shared/disk.cpp $(INIT_BUILD_DIRS)
	$(_CC16) $< -o $@ $(CC_INCLUDE_BOOT) -include src/boot/shared/include/aux.h

temp/boot/%.cpp.elf16: src/boot/%.cpp $(INIT_BUILD_DIRS)
	$(_CXX16) $< -o $@ $(CC_INCLUDE_BOOT)

temp/boot/%.c.elf32: src/boot/%.c $(INIT_BUILD_DIRS)
	$(_CC32) $< -o $@ $(CC_INCLUDE_BOOT)

temp/boot/%.cpp.elf32: src/boot/%.cpp $(INIT_BUILD_DIRS)
	$(_CXX32) $< -o $@ $(CC_INCLUDE_BOOT)

temp/boot/%.c.elf64: src/boot/%.c $(INIT_BUILD_DIRS)
	$(_CC64) $< -o $@ $(CC_INCLUDE_BOOT)

temp/boot/%.cpp.elf64: src/boot/%.cpp $(INIT_BUILD_DIRS)
	$(_CXX64) $< -o $@ $(CC_INCLUDE_BOOT)



debug32_qemu:
	$(QEMU32) $(QEMU_TEMPDBG_ARGS) &
	$(GDB32) || true



debug64_qemu:
	$(QEMU64) $(QEMU_TEMPDBG_ARGS) &
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
	rm -rf $(SYSTEM_DRIVES)
	mkdir -p system

	dd if=/dev/zero of=$(SYSTEM_DRIVE1_FILE) bs=1M count=$(SYSTEM_DRIVE1_SIZE)
	VBoxManage internalcommands createrawvmdk -filename $(SYSTEM_DRIVE1_FILE_VB) -rawdisk "$(LOCAL_PATH)/$(SYSTEM_DRIVE1_FILE)" || true

	dd if=/dev/zero of=$(SYSTEM_DRIVE2_FILE) bs=1M count=$(SYSTEM_DRIVE2_SIZE)
	VBoxManage internalcommands createrawvmdk -filename $(SYSTEM_DRIVE2_FILE_VB) -rawdisk "$(LOCAL_PATH)/$(SYSTEM_DRIVE2_FILE)" || true



$(PARTITIONER_MBR): $(PARTITIONER_MBR).cpp
	$(_CXX) $< -o $@ -g $(CC_INCLUDE_BOOT)
$(BURNER): $(BURNER).cpp
	$(_CXX) $< -o $@ -g $(CC_INCLUDE_BOOT) src/boot/shared/disk.cpp -include string.h



system_reset_mbr: $(PARTITIONER)
	$(PARTITIONER) $(SYSTEM_DRIVE1_FILE) 1 $(MBR_SYSTEM_PARTITION_TYPE) -1 -1 0



system_burn_mbr: all
	mkdir -p system
	dd bs=1 if=$(STOS_LOADER_MBR_RAW) of=$(SYSTEM_DRIVE1_FILE) conv=notrunc count=440 #Copy initial data upto the partition table
	dd bs=1 if=$(STOS_LOADER_MBR_RAW) of=$(SYSTEM_DRIVE1_FILE) conv=notrunc seek=510 skip=510 count=2 #Copy MBR marker
	dd bs=1 if=$(STOS_LOADER_MBR_RAW) of=$(SYSTEM_DRIVE1_FILE) conv=notrunc seek=512 skip=512 #Copy MBR second stage
