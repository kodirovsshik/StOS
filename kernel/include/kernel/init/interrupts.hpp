
#ifndef _KERNEL_INIT_INTERRUPTS_HPP_
#define _KERNEL_INIT_INTERRUPTS_HPP_

void invalid_opcode_handler();
void page_fault_handler();
void general_protection_fault_handler();
void double_fault_handler();

void initialize_interrupt_handling();

#endif //!_KERNEL_INIT_INTERRUPTS_HPP_
