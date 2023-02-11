
#include "kernel/interrupts.hpp"
#include "kernel/init/interrupts.hpp"

#include <stdint.h>


void initialize_interrupt_handling()
{
	extern idt_t idt;
	idt[0x06].fill_trap_entry((void*)invalid_opcode_handler);
	idt[0x08].fill_trap_entry((void*)double_fault_handler);
	idt[0x0D].fill_trap_entry((void*)general_protection_fault_handler);
	idt[0x0E].fill_trap_entry((void*)page_fault_handler);
	
	extern idtr_t idtr;
	asm volatile("lidt [%[idtr]]" :: [idtr] "m" (idtr));
}


extern "C" [[noreturn]]
void _halt();


void print_excp_status(const char* str)
{
	*(uint16_t*)0xb8002 = 0x0400 + '#';
	*(uint16_t*)0xb8004 = 0x0400 + str[0];
	*(uint16_t*)0xb8006 = 0x0400 + str[1];
}

void invalid_opcode_handler()
{
	print_excp_status("UD");
	_halt();
}
void page_fault_handler()
{
	print_excp_status("PF");
	_halt();
}
void general_protection_fault_handler()
{
	print_excp_status("GP");
	_halt();
}
void double_fault_handler()
{
	print_excp_status("DF");
	_halt();
}
