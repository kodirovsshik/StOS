
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

struct loader_data_t
{
	uint8_t boot_disk_uuid[16];
	uint64_t boot_partition_lba;
	uint32_t memory_map_addr;
	uint16_t memory_map_size;
	uint16_t output_buffer_index;
};

extern "C" [[noreturn]]
void kmain(loader_data_t* loader_data)
{
	(void)loader_data;
	cls();
	print_at("In kernel", 1, 1);
	halt();
}
