
#ifndef _INTERRUPTS_HPP_
#define _INTERRUPTS_HPP_


#include <stdint.h>

struct __attribute__((packed)) idt_entry_t
{
	uint16_t offset1;
	uint16_t selector;
	uint8_t interrupt_stack_table_offset : 3;
	uint8_t _reserved1 : 5;
	uint8_t gate_type : 4;
	uint8_t _reserved2 : 1;
	uint8_t privilege_level : 2;
	bool present : 1;
	uint16_t offset2;
	uint32_t offset3;
	uint32_t _reserved3;

	void fill_offset(const void* isr);
	void fill_trap_entry(const void* isr, uint8_t ist = 0);
};
static_assert(sizeof(idt_entry_t) == 16, "");

using idt_t = idt_entry_t[256];



struct __attribute__((packed)) idtr_t
{
	uint16_t size;
	uint64_t address;
};

#define IDT_GATE_TYPE_INTERRUPT 0xE
#define IDT_GATE_TYPE_TRAP 0xF


#endif //!_INTERRUPTS_HPP_
