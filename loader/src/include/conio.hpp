
#ifndef _CONIO_HPP_
#define _CONIO_HPP_

#include <stdint.h>
#include "farptr.hpp"

struct _conio_params
{
	bool& use_screen;
	bool& use_memory_buffer;
	bool& use_serial_port;
};
static_assert(sizeof(_conio_params) == 3 * sizeof(void*), "");

extern "C"
{
	void cputc(char);
	void cputs(FarPtr<const char>, bool break_line = false);
	void cput32x(uint32_t value, uint16_t min_digits = 0);
	void cput32u(uint32_t value, uint16_t min_digits = 0);
	void flush_io_memory_buffer();
}

extern "C"
void __fill_conio_params(void*);

static _conio_params _get_conio_params()
{
	alignas(void*) unsigned char params[sizeof(_conio_params)];
	__fill_conio_params(&params);
	return *(_conio_params*)&params;
}
static void cputs(const char* ptr, bool ln = false)
{
	cputs(FarPtr<const char>::convert(ptr), ln);
}
static void cput64x(uint64_t value, uint16_t min_digits = 0)
{
	uint16_t min_digits_high = (min_digits >= 8) ? (min_digits - 8) : 0;
	uint32_t value_high = (uint32_t)(value >> 32);
	if (value_high || min_digits_high)
		cput32x(value_high, min_digits_high);
	cput32x((uint32_t)value, min_digits - min_digits_high);
}
	

#define conio _get_conio_params()

#endif //!_CONIO_HPP_
