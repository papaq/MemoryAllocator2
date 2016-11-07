// MemoryAllocatorInPages.c : Defines the entry point for the console application.
//

#include "Allocator.h"
#include "ConsoleOutput.h"
#include <stdlib.h>

int main()
{
	print_info();
	global_mem = malloc(MEM_SIZE);
	
	init_pages();

	mem_dump();

	printf("*** c = mem_alloc(1555) ***\n");
	void * c = mem_alloc(1555);	
	mem_dump();

	printf("*** c = mem_realloc(c, 3572) ***\n");
	c = mem_realloc(c, 3572);
	mem_dump();

	printf("*** mem_free(c) ***\n");
	mem_free(c);
	mem_dump();

    return 0;
}

