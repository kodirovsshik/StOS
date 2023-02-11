
#include "kernel/interrupts.hpp"
#include "kernel/aux/memset.hpp"


idt_t idt;

idtr_t idtr{ sizeof(idt), (uint64_t)&idt };


void idt_entry_t::fill_offset(const void* isr)
{
	uint64_t ptr = (uint64_t)isr;
	this->offset1 = uint16_t(ptr >> 0);
	this->offset2 = uint16_t(ptr >> 16);
	this->offset3 = uint32_t(ptr >> 32);
}
void idt_entry_t::fill_trap_entry(const void* isr, uint8_t ist)
{
	memset(this, 0, sizeof(*this));
	this->fill_offset(isr);
	this->gate_type = IDT_GATE_TYPE_TRAP;
	this->interrupt_stack_table_offset = ist;
	this->selector = 0x8;
	this->present = true;
}
