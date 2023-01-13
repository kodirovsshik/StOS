
override MBR_SRC_DIR := $(MBR_DIR)/src
override MBR_SRC := $(MBR_SRC_DIR)/mbr.asm

$(MBR_BIN): $(MBR_SRC)
	$(NASM) $^ -o $@ -f bin
	chmod a-x $@

clean-sub-$(MBR_DIR):
	rm -f $(MBR_BIN)
