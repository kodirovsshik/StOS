
#include <stdint.h>
#include <stddef.h>

#include "include/interrupt.hpp"
#include "include/loader_common.hpp"
#include "include/ram.hpp"

extern "C" void* memset(void* dst, uint8_t val, size_t size);
extern "C" void* memcpy(void* dst, const void* src, size_t size);

struct sector_listing_data
{
	uint32_t sectors[101];
	uint8_t counts[101];
	uint8_t _pad[3];
	uint32_t next;
};
static_assert(sizeof(sector_listing_data) == 512);

struct lba_packet
{
	uint8_t size = 16;
	uint8_t _reserved1 = 0;
	uint8_t count;
	uint8_t _reserved2 = 0;
	uint16_t offset;
	uint16_t segment;
	uint64_t lba;
};
static_assert(sizeof(lba_packet) == 16);

struct alignas(8) edata_t
{
	uint8_t boot_disk_uuid[16];
	uint64_t boot_partition_lba;
	uint32_t memory_map_addr;
	uint16_t memory_map_size;
	uint16_t output_buffer_index;
};
extern "C" edata_t edata;

uint8_t disk_read(uint64_t lba, uint8_t count, void* ptr, uint8_t disk = 0xFF);
uint8_t boot_partition_read(uint32_t sector, uint8_t count, void* ptr);

extern "C" uint32_t kernel_listing_sector;

union
{
	uint64_t value = 0;
	struct
	{
		uint8_t io_status;
		uint8_t last_read_size;
		uint8_t _pad0;
		uint8_t _pad1 : 7;
		bool error : 1;
		uint32_t last_read_sector;
	};
} return_data;
static_assert(sizeof(return_data) == 8);





struct
{
	uint32_t free_paging_area;
	uint32_t page_map_base;

	uint32_t alloc()
	{
		uint32_t result = free_paging_area;
		memset((void*)result, 0, 4096);
		free_paging_area += 4096;
		return result;
	}
} paging_data;

struct page_map
{
	uint64_t entries[512];
};
static_assert(sizeof(page_map) == 4096);



#define MMAP_PRESENT (1 << 0)
#define MMAP_WRITABLE (1 << 1)
#define MMAP_WRITETHROUGH (1 << 3)
#define MMAP_NOT_CACHABLE (1 << 4)
#define MMAP_ATTRIBS_MASK 0xFFE

void mmap(uint64_t physical, uint64_t virtual_, uint16_t attribs = MMAP_WRITABLE)
{
	uint32_t current_address = paging_data.page_map_base;
	for (int off = 39; off >= 21; off -= 9)
	{
		const uint32_t index = (virtual_ >> off) & 511;
		auto ptr = (page_map*)current_address;
		auto& next_addr = ptr->entries[index];
		if ((next_addr & MMAP_PRESENT) == 0)
		{
			next_addr = paging_data.alloc();
			next_addr |= MMAP_PRESENT | MMAP_WRITABLE;
		}
		//1 top and 12 lower bits are flags
		current_address = (uint32_t)(next_addr & 0x7FFFFFFFFFFFF000u);
	}
	const uint32_t index = (virtual_ >> 12) & 511;
	auto& entry = ((page_map*)current_address)->entries[index];
	entry = physical & ~(uint64_t)4095;

	attribs &= MMAP_ATTRIBS_MASK;
	attribs |= MMAP_PRESENT;
	entry |= attribs;
}

void setup_paging_initial()
{
	paging_data.page_map_base =
	paging_data.free_paging_area = 
		0x30000;
	
	paging_data.alloc();
	
	//Ordinary low memory
	for (int i = 0; i < 128; ++i)
		mmap(4096 * i, 4096 * i);
	//Video memory
	for (int i = 160; i < 192; ++i)
		mmap(4096 * i, 4096 * i, MMAP_WRITABLE | MMAP_WRITETHROUGH);
}

uint32_t next_kernel_load_page()
{
	static uint16_t current_entry = 0;
	for (uint16_t i = current_entry; i < edata.memory_map_size; ++i)
	{
		current_entry = i;
		auto& entry = ((memory_map_entry_t*)edata.memory_map_addr)[i];
		if (entry.begin < 0x100000)
			continue;
		if (entry.begin == entry.end)
			continue;
		if (entry.begin > UINT32_MAX)
			return 0;
		auto addr = (uint32_t)entry.begin;
		entry.begin += 4096;
		return addr;
	}
	return 0;
}

void load_kernel()
{
	uint64_t kernel_virtual_addr = 0xFFFFFFFF80000000u;
	uint32_t addr;
	uint8_t sectors_left_in_page;
	auto new_page = [&]
	{
		addr = next_kernel_load_page();
		sectors_left_in_page = 8;
	};

	uint32_t sector;
	uint8_t sectors_count = 0;
	auto read = [&](uint8_t sectors)
	{
		boot_partition_read(sector, sectors, (void*)addr);
		sectors_left_in_page -= sectors;
		sectors_count -= sectors;
		addr += sectors * 512;
		sector += sectors;
	};

	sector_listing_data listing;
	uint32_t listing_next_sector = kernel_listing_sector;
	uint8_t listing_index = arr_size(listing.sectors);
	auto update_listing = [&]
	{
		boot_partition_read(listing_next_sector, 1, &listing);
		listing_index = 0;
		listing_next_sector = listing.next;
	};

	auto fetch = [&]
	{
		if (listing_next_sector == 0)
		{ //end reached
			sectors_count = 0;
			return true; 
		}
		if (listing_index == arr_size(listing.sectors))
			update_listing();
		
		if (return_data.io_status != 0)
			return false;

		sector = listing.sectors[listing_index];
		sectors_count = listing.counts[listing_index];
		++listing_index;
		return true;
	};

	while (true)
	{
		new_page();
		while (true)
		{
			if (sectors_count == 0 && !fetch())
				return;
			if (sectors_count == 0)
				break;

			if (sectors_count > sectors_left_in_page)
				break;
			read(sectors_count);
			if (return_data.io_status != 0)
				return;
		}
		mmap(addr, kernel_virtual_addr);
		if (sectors_count == 0)
			break;

		kernel_virtual_addr += 4096;

		read(sectors_left_in_page);
		if (return_data.io_status != 0)
			return;

	}
}



struct idt_entry
{
	uint16_t offset1;
	uint16_t selector = 8;
	uint8_t _reserved0 = 0;
	uint8_t type : 4 = 0xF;
	bool _reserved1 : 1 = 0;
	uint8_t dbp : 2 = 0;
	bool present : 1 = 0;
	uint16_t offset2;
	uint32_t offset3;
	uint32_t _reserved2 = 0;
};
static_assert(sizeof(idt_entry) == 16);

extern "C" idt_entry idt64[32];
extern "C" uint32_t idt_handlers[32];
void fill_idt()
{
	for (int i = 0; i < 32; ++i)
	{
		auto& entry = idt64[i];
		entry = idt_entry{};

		uint32_t isr = idt_handlers[i];
		if (isr == 0)
			continue;

		entry.present = 1;
		entry.offset1 = (uint16_t)isr;
		entry.offset2 = (uint16_t)(isr >> 16);
		entry.offset3 = 0;
	}
}



extern "C"
uint64_t pm_main()
{
	setup_paging_initial();
	load_kernel();
	if (return_data.io_status != 0)
		return_data.error = true;
	fill_idt();
	return return_data.value;
}


uint8_t boot_partition_read(uint32_t sector, uint8_t count, void* ptr)
{
	return_data.last_read_sector = sector;
	return_data.last_read_size = count;
	return disk_read(edata.boot_partition_lba + sector, count, ptr);
}

extern "C" uint8_t pbr_disk;
uint8_t disk_read(uint64_t lba, uint8_t count, void* ptr, uint8_t disk)
{
	if (disk == 0xFF)
		disk = pbr_disk;
	lba_packet data;
	data.count = count;
	data.offset = 0;
	data.segment = 0x2000;
	data.lba = lba;

	regs_t regs{};
	regs.ax = 0x4200;
	regs.si = ptr_cast(uint16_t, &data);
	regs.dl = pbr_disk;
	interrupt(&regs, 0x13);

	return_data.io_status = regs.ah;

	memcpy(ptr, (void*)0x20000, data.count * (size_t)512);
	return data.count;
}

#include "include/memfunc.cpp"
