
override BINECHO_SRC_DIR := $(BINECHO_DIR)/src
override BINECHO_SRC := $(BINECHO_SRC_DIR)/binecho.cpp

$(BINECHO_BIN): $(BINECHO_SRC)
	$(_CXX) $^ -o $@

clean-sub-$(BINECHO_DIR):
	rm -f $(BINECHO_BIN)