// MemoryAllocatorInPages.c : Defines the entry point for the console application.
//

#include "Allocator.h"
#include "ConsoleOutput.h"
#include <stdlib.h>

int main()
{
	print_info();
	global_mem = malloc(MEM_SIZE);

	int * a = global_mem;

	init_pages();

	mem_dump();

    return 0;
}

