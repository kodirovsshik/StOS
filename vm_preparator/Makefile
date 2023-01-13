
override VM_PREPARATOR_SRC_DIR := $(VM_PREPARATOR_DIR)/src
override VM_PREPARATOR_SRC := $(VM_PREPARATOR_SRC_DIR)/vm_preparator.cpp

$(VM_PREPARATOR_BIN): $(VM_PREPARATOR_SRC)
	$(_CXX) -O0 $^ -o $@ -std=c++20
#No optimization because the compilation already runs long enough

clean-sub-$(VM_PREPARATOR_DIR):
	rm -f $(VM_PREPARATOR_BIN)
