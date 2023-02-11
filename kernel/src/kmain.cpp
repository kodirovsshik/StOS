
#include <stdint.h>


extern "C" [[noreturn]]
void _halt();


extern "C" [[noreturn]]
void kmain()
{
	_halt();
}
