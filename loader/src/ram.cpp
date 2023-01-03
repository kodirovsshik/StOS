
#include "include/loader.hpp"
#include "include/interrupt.hpp"
#include "include/alloc.hpp"
#include "include/conio.hpp"

void fill_memory_table();
void fill_memory_table_fallback();
void sort_memory_table();
void print_memory_table();
void ensure_kernel_load_memory_usable();
void try_fill_memory_table_e820();
void get_upper_memory_data_e881(uint32_t& at1m, uint64_t& at16m);
void get_lower_memory_data_int12(uint32_t&);
[[noreturn]] void error_memory_detection();


struct memory_map_entry_t
{
	uint64_t begin, end;
	uint32_t type;
	uint32_t flags;
};
static_assert(sizeof(memory_map_entry_t) == 24, "");

#define memory_map_addr c_get_memory_map_addr()
#define memory_map_size c_get_memory_map_size()

extern "C" memory_map_entry_t*& c_get_memory_map_addr();
extern "C" uint16_t& c_get_memory_map_size();


extern "C"
void do_subtask_memory()
{
	fill_memory_table();
	if (memory_map_size == 0)
		error_memory_detection();
	sort_memory_table();
	//resolve_memory_table_overlaps();
	print_memory_table();
	ensure_kernel_load_memory_usable();
}

void sort_memory_table()
{
	//sorts memory table by ranges' begins
	//uses selection sort because it is stupidly easy to write
	const auto n = memory_map_size;
	auto* table = memory_map_addr;
	for (uint16_t i = 0; i < n; ++i)
	{
		uint16_t min = i;
		for (uint16_t j = i + 1; j < n; ++j)
		{
			if (table[j].begin < table[min].begin)
				min = j;
		}
		swap(table[i], table[min]);
	}
}

void print_memory_table()
{
	const auto n = memory_map_size;
	const auto* table = memory_map_addr;
	for (uint16_t i = 0; i < n; ++i)
	{
		const auto& entry = table[i];
		cput64x(entry.begin, 16);
		cputc(' ');
		cput64x(entry.end, 16);
		cputc(' ');
		cput32u(entry.type);
		cputc(' ');
		cput32x(entry.flags, 2);
		cputc('\n');
	}
}

//Ensure memory region [1 MiB; 1 MiB + delta) exists and is available
void ensure_kernel_load_memory_usable()
{
	const uint32_t kernel_minimum_memory = 64 * KiB;

	//To be moved to last available 32-bit address in a region starting at 1 MiB
	uint32_t low = MiB;
	const uint32_t high = low + kernel_minimum_memory;

	const auto n = memory_map_size;
	const auto* table = memory_map_addr;
	for (uint16_t i = 0; i < n; ++i)
	{
		auto& entry = table[i];
		if (entry.begin > UINT32_MAX)
			break;
		if (low >= high)
			break;
		if (entry.type != 1)
			continue;
		if (low >= entry.begin && low < entry.end)
			low = (uint32_t)min<uint64_t>(entry.end, UINT32_MAX);
	}
	cputs("Available memory at 1 MiB: ");
	cput32u((low - MiB + 512) / KiB);
	cputs(" KiB\n");
	if (low < high)
		cpanic("Not enough memory available at 1 MiB");
}


void fill_memory_table()
{
	try_fill_memory_table_e820();
	if (memory_map_size == 0)
		fill_memory_table_fallback();
}
void fill_memory_table_fallback()
{
	uint32_t memory_at_0;
	uint32_t memory_at_1m;
	uint64_t memory_at_16m;
	get_lower_memory_data_int12(memory_at_0);
	get_upper_memory_data_e881(memory_at_1m, memory_at_16m);

	constexpr uint32_t MiB = 1024 * 1024;

	auto insert_memory_table_entry = []
	(uint64_t begin, uint64_t size)
	{
		if (size == 0)
			return;
		auto& entry = memory_map_addr[memory_map_size++];
		entry.begin = begin;
		entry.end = begin + size;
		entry.type = entry.flags = 1;
	};

	memory_map_addr = (memory_map_entry_t*)c_heap_get_ptr();

	insert_memory_table_entry(0, memory_at_0);
	insert_memory_table_entry(MiB, memory_at_1m);
	insert_memory_table_entry(16 * MiB, memory_at_16m);

	c_heap_set_ptr(memory_map_addr + memory_map_size);
}
void try_fill_memory_table_e820()
{
	memory_map_entry_t entry{};
	regs_t regs{};
	regs.di = ptr_cast(uint16_t, &entry);
	uint32_t index = 0;

	memory_map_addr = (memory_map_entry_t*)c_heap_get_ptr();

	while (true)
	{
		regs.eax = 0xE820;
		regs.edx = 0x534D4150;
		regs.ecx = 24;
		regs.ebx = index;
		interrupt(&regs, 0x15);
		index = regs.ebx;

		if (regs.eax != 0x534D4150)
			break;
		if (regs.flags & EFLAGS_CARRY)
			break;
		if (regs.cx < 24)
			entry.flags = 1;
		if ((entry.flags & 1) == 0 || (entry.flags & 2))
			continue;
		
		entry.end += entry.begin;
		memcpy(memory_map_addr + memory_map_size++, &entry, sizeof(memory_map_entry_t));

		if (index == 0)
			break;
	}

	c_heap_set_ptr(memory_map_addr + memory_map_size);
}
void get_upper_memory_data_e881(uint32_t& at1m, uint64_t& at16m)
{
	regs_t regs{};
	regs.eax = 0xE881;
	interrupt(&regs, 0x15);
	if (regs.flags & EFLAGS_CARRY)
	{
		regs.ax = 0xE801;
		interrupt(&regs, 0x15);
		if (regs.flags & EFLAGS_CARRY)
			return;
		//Contents of high words is not defined for this interrupt
		regs.eax = regs.ax;
		regs.ecx = regs.cx;
		regs.edx = regs.dx;
		regs.ebx = regs.bx;
	}
	
	if (regs.ecx != 0)
	{
		regs.eax = regs.ecx;
		regs.ebx = regs.edx;
	}

	at1m = regs.eax * 1024;
	at16m = (uint64_t)regs.ebx * 65536;
}
void get_lower_memory_data_int12(uint32_t& mem)
{
	regs_t regs{};
	interrupt(&regs, 0x12);
	if (regs.flags & EFLAGS_CARRY)
		regs.eax = 0;
	mem = regs.eax * 1024;
}

[[noreturn]]
void error_memory_detection()
{
	cpanic("Memory detection failure");
}
