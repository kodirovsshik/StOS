
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
