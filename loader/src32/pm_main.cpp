
#include <stdint.h>
#include <stddef.h>

extern "C" void outb(uint16_t port, uint8_t value);
extern "C" uint8_t inb(uint16_t port);

extern "C" void* memset(void* dst, uint8_t val, size_t size);

void outsb(uint16_t port, const char* p)
{
	while (*p)
		outb(port, *p++);
}

extern "C"
void pm_main()
{
}
