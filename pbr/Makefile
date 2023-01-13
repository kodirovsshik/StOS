
override PBR_SRC_DIR := $(PBR_DIR)/src
override PBR_SRC := $(PBR_SRC_DIR)/pbr.asm

$(PBR_BIN): $(PBR_SRC)
	$(NASM) $^ -o $@ -f bin
	chmod a-x $@

clean-sub-$(PBR_DIR):
	rm -f $(PBR_BIN)
