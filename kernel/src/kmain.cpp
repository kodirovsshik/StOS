
#include <stdint.h>


extern "C" [[noreturn]]
void halt();

void cls()
{
	auto ptr = (uint16_t*)0xb8000ull;
	for (int i = 0; i < 80*25; ++i)
		*ptr++ = 0;
}

void print_at(const char* s, int row, int column, int color = 0x02)
{
	char* ptr = (char*)(0xb8000ull + (row * 80 + column) * 2);
	while (*s)
	{
		*ptr++ = *s++;
		*ptr++ = color;
	}
}

extern "C" [[noreturn]]
void kmain()
{
	cls();
	print_at("In kernel", 1, 1);
	halt();
}
