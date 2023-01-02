
#ifndef _INTERRUPT_HPP_
#define _INTERRUPT_HPP_

#include <stdint.h>

#define DEFINE_REG(midch, lowch) \
union\
{\
	uint32_t e ## midch ## lowch;\
	uint16_t midch ## lowch;\
	struct\
	{\
		uint8_t midch ## l;\
		uint8_t midch ## h;\
	};\
};

struct regs_t
{
	DEFINE_REG(a, x);
	DEFINE_REG(c, x);
	DEFINE_REG(d, x);
	DEFINE_REG(b, x);
	DEFINE_REG(si,);
	DEFINE_REG(di,);
	DEFINE_REG(bp,);
	DEFINE_REG(flags,);
};
static_assert(sizeof(regs_t) == 32);

#undef DEFINE_REG

#define EFLAGS_CARRY 0x00000001

extern "C" void interrupt(regs_t*, uint8_t vector);

#endif //!_INTERRUPT_HPP_
