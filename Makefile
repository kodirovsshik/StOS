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



NASM := nasm -Wall -Wother -Werror -Wno-label-redef-late -Wno-gnu-elf-extensions -gdwarf

ifeq ($(DEBUG),no)
override DBG := -Os
else
override DBG := -g
endif

override _CC_ARGS := $(CC_ARGS) -c -ffreestanding -mno-red-zone -Wall -Wextra -Werror $(DBG) -I$(LOCAL_PATH)/src/include
override _CXX_ARGS := $(CXX_ARGS) $(_CC_ARGS) -fno-rtti -fno-exceptions -std=c++20

CC64 := x86_64-elf-gcc
CC32 := i686-elf-gcc
CC16 := $(CC32) -m16

CXX64 := x86_64-elf-g++
CXX32 := i686-elf-g++
CXX16 := $(CXX32) -m16

CC := gcc
CXX := g++

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



BOOTLOADER_OBJS :=
BOOTLOADER_OBJS += temp/bootloader/main.cpp.elf16
BOOTLOADER_OBJS += temp/bootloader/io.cpp.elf16
BOOTLOADER_OBJS += temp/bootloader/disk.cpp.elf16
BOOTLOADER_OBJS += temp/bootloader/bootloader.cpp.elf16
BOOTLOADER_OBJS += temp/bootloader/entry.asm.elf32
BOOTLOADER_OBJS += temp/bootloader/interrupt.asm.elf32
BOOTLOADER_OBJS += temp/bootloader/io.asm.elf32
BOOTLOADER_OBJS += temp/bootloader/aux.asm.elf32

BOOTLOADER_MBR_RAW := result/boot_mbr.img
BOOTLOADER_MBR_ELF := result/boot_mbr.elf



PARTITIONER := stuff/partitioner/a.out





CREATE_OUTPUT_DIR = X="$@" ; mkdir -p $${X%/*}





.PHONY: all clean bootloader loader rebuild host_boot host_boot_debug
.PHONY: debug32_qemu debug64_qemu system_wipe system_burn_mbr
.PHONY: run32 run32v run64 run64v



all: bootloader loader



rebuild: clean all



bootloader: $(BOOTLOADER_MBR_RAW)



loader:



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



$(BOOTLOADER_MBR_RAW): $(BOOTLOADER_OBJS)
	$(CREATE_OUTPUT_DIR)
	$(LINK32) -T src/link/bootmgr.ld $(BOOTLOADER_OBJS) -o $(BOOTLOADER_MBR_ELF)
	$(OBJCOPY32) -O binary $(BOOTLOADER_MBR_ELF) $(BOOTLOADER_MBR_RAW)
	ndisasm -o 0x400 $(BOOTLOADER_MBR_RAW) > result/bootloader_mbr_disasm
	xxd $(BOOTLOADER_MBR_RAW) > result/bootloader_mbr_hexdump



clean:
	rm -rf temp/* result/*
	mkdir -p temp/bootloader



clean_wipe:
	rm -rf system temp result
	rm -f $(PARTITIONER)



temp/%.asm.elf32: src/%.asm
	$(CREATE_OUTPUT_DIR)
	$(NASM) -f elf32 $< -o $@

temp/%.asm.elf64: src/%.asm
	$(CREATE_OUTPUT_DIR)
	$(NASM) -f elf64 $< -o $@

temp/%.c.elf16: src/%.c
	$(CREATE_OUTPUT_DIR)
	$(_CC16) $< -o $@

temp/%.cpp.elf16: src/%.cpp
	$(CREATE_OUTPUT_DIR)
	$(_CXX16) $< -o $@

temp/%.c.elf32: src/%.c
	$(CREATE_OUTPUT_DIR)
	$(_CC32) $< -o $@

temp/%.cpp.elf32: src/%.cpp
	$(CREATE_OUTPUT_DIR)
	$(_CXX32) $< -o $@

temp/%.c.elf64: src/%.c
	$(CREATE_OUTPUT_DIR)
	$(_CC64) $< -o $@

temp/%.cpp.elf64: src/%.cpp
	$(CREATE_OUTPUT_DIR)
	$(_CXX64) $< -o $@



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



stuff/%.out: stuff/%.cpp
	$(CC) $< -o $@ -g
stuff/%.out: stuff/%.c
	$(CC) $< -o $@ -g



system_reset_mbr: $(PARTITIONER)
	$(PARTITIONER) $(SYSTEM_DRIVE1_FILE) 1 $(MBR_SYSTEM_PARTITION_TYPE) -1 -1 0



system_burn_mbr: all
	mkdir -p system
	dd bs=1 if=$(BOOTLOADER_MBR_RAW) of=$(SYSTEM_DRIVE1_FILE) conv=notrunc count=440 #Copy initial data upto the partition table
	dd bs=1 if=$(BOOTLOADER_MBR_RAW) of=$(SYSTEM_DRIVE1_FILE) conv=notrunc seek=510 skip=510 count=2 #Copy MBR marker
	dd bs=1 if=$(BOOTLOADER_MBR_RAW) of=$(SYSTEM_DRIVE1_FILE) conv=notrunc seek=512 skip=512 #Copy MBR second stage
