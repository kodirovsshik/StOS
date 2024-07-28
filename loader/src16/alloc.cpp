
#include <stdint.h>

extern "C" const char loader_end;

static uint16_t heap_ptr = (uint16_t)(uintptr_t)&loader_end;

void* heap_get_ptr()
{
	return (void*)(uintptr_t)heap_ptr;
}

void heap_set_ptr(void* p)
{
	heap_ptr = (uint16_t)(uintptr_t)p;
}
