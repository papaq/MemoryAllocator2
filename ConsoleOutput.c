#include "ConsoleOutput.h"

void mem_dump()
{
	for (int i = 0; i < PAGE_NUM; i++)
	{
		int curr_descriptor = descriptors[i];
		// If non-class:
		if (!get_type_class(curr_descriptor))
		{
			printf("Page #%d\ntype: non-class\n", get_page_num(curr_descriptor));
			if (get_busy(curr_descriptor))
			{
				printf("busy: yes\n");
			}
			else
			{
				printf("busy: no\n");
			}
			printf("pages in block: %d\n", get_pages_in_block(curr_descriptor));
			printf("\n");
			continue;
		}

		// If class:
		printf("Page #%d\ntype: class\n", get_page_num(curr_descriptor));

		int class_size = get_size_of_class(curr_descriptor);

		printf("Class size: %d\n", class_size);
		printf("Left free classes: %d", get_left_blocks(curr_descriptor));

		//int * ptr_to_free = get_pointer_to_block(curr_descriptor);
		int * ptr_to_block = (int *)global_mem + i * PAGE_SIZE / sizeof(int);

		// Print all blocks
		for (int i = 0; i < PAGE_SIZE / class_size; i++)
		{
			// If the first int val is a header, then the block is busy
			if (get_head_bit(*ptr_to_block) == 1)
			{
				printf("block #%d\n", i);
				printf("busy: yes\n");
				printf("     %d%c %d%c %d%c\n", sizeof(int), HEAD,
					get_busy_bytes(*ptr_to_block), BUSYBL,
					class_size - sizeof(int) - get_busy_bytes(*ptr_to_block), EMPTYBL);

				ptr_to_block += class_size / sizeof(int);
				continue;
			}
			printf("block #%d\n", i);
			printf("busy: no\n");

			ptr_to_block += class_size / sizeof(int);
		}

		printf("\n");
	}
}

void print_info()
{
	printf("**** Page size: %d **** Pages: %d ****\n\n", PAGE_SIZE, PAGE_NUM);
}

