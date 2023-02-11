
#include <stdint.h>
#include "kernel/aux/memset.hpp"
#include "kernel/interrupts.hpp"
#include "kernel/memory.hpp"
#include "kernel/init/init.hpp"
#include "kernel/init/interrupts.hpp"



extern "C" uint8_t _kernel_code_begin;
extern "C" uint8_t _kernel_bss_begin;
extern "C" uint8_t _kernel_data_begin;
extern "C" uint8_t _kernel_rodata_begin;

extern "C" uint8_t _kernel_code_size_pages;
extern "C" uint8_t _kernel_bss_size_pages;
extern "C" uint8_t _kernel_data_size_pages;
extern "C" uint8_t _kernel_rodata_size_pages;

extern "C" uint8_t _kernel_bss_end;


#define remap_memory(begin, pages, flags) \
	update_virtual_range_flags((uint64_t)&begin, (uint32_t)(uint64_t)&pages, flags | MMAP_GLOBAL)

extern uint64_t _paging_memory_manager_top;

void update_status(uint8_t value)
{
	*(uint8_t*)0xb8000 = value;
}

extern "C"
void kinit(loader_data_t* loader_data)
{
	(void)loader_data;

	_paging_memory_manager_top = loader_data->free_paging_area;

	memset((void*)0xb8000, 0, 80*25*2);
	*(uint16_t*)0xb8000 = 0x0200;
	update_status('1');

	remap_memory(_kernel_code_begin, _kernel_code_size_pages, 0);
	remap_memory(_kernel_bss_begin, _kernel_bss_size_pages, MMAP_WRITABLE | MMAP_UNEXECUTABLE);
	remap_memory(_kernel_data_begin, _kernel_data_size_pages, MMAP_WRITABLE | MMAP_UNEXECUTABLE);
	remap_memory(_kernel_rodata_begin, _kernel_rodata_size_pages, MMAP_UNEXECUTABLE);
	
	update_status('2');

	memset(&_kernel_bss_begin, 0, &_kernel_bss_end - &_kernel_bss_begin);
	
	update_status('3');

	initialize_interrupt_handling();

	update_status('~');
	nop();
}
