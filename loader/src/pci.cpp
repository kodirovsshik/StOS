
#include "include/conio.hpp"
#include <stdint.h>

#define _str(x) #x
#define _str2(x) _str(x)
#define _GETLINE _str2(__LINE__)
#define assert(cond) { if (!(cond)) { cpanic(__FILE__ ":" _GETLINE ": assertion failed: " #cond); } }

extern "C" void outd(uint16_t port, uint32_t data);
extern "C" uint32_t ind(uint16_t port);

uint32_t pci_read32_aligned(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off)
{
	assert((dev & 0b11100000) == 0);
	assert((func & 0b11111000) == 0);
	assert((off & 0b00000011) == 0);
	using ui32 = uint32_t;
	outd(0x0CF8, (ui32(1) << 31) | (ui32(bus) << 16) | (ui32(dev) << 11) | (ui32(func) << 8) | off);
	return ind(0x0CFC);
}
uint16_t pci_read16_aligned(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off)
{
	uint32_t data32 = pci_read32_aligned(bus, dev, func, off & ~2);
	return (uint16_t)(data32 >> (16 * bool(off & 2)));
}

extern "C"
void do_pci()
{
	bool saved_use_screen = conio.use_screen;
	conio.use_screen = true;
	for (int bus = 0; bus < 256; ++bus)
	{
		for (uint8_t dev = 0; dev < 32; ++dev)
		{
			for (uint8_t func = 0; func < 8; ++func)
			{
				uint16_t vendor_id = pci_read16_aligned(bus, dev, func, 0);
				uint16_t device_id = pci_read16_aligned(bus, dev, func, 2);
				if (vendor_id == 0xFFFF || device_id == 0xFFFF)
					continue;
				cputs("bus ");
				cput32u(bus, 3);
				cputs(", device ");
				cput32u(dev, 2);
				cputs(", function ");
				cput32u(func);
				cputs(": Vendor=");
				cput32x(vendor_id, 4);
				cputs(", device=");
				cput32x(device_id, 4);
				cputc('\n');
			}
		}
	}
	conio.use_screen = saved_use_screen;
}
