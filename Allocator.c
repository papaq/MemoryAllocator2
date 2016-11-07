#include "Allocator.h"

#include <math.h>

int make_descriptor(bool class_type, int num);
int make_class_header(int class_size, int busy_bytes, int num);

int fill_class_descriptor(int num, int size);

int fill_nonclass_descriptor(int num, int pages_in_block, bool busy);


void set_head_bit(int * value);
void set_page_num(int * descr, int num);
void set_type_class(int * descr, bool class_type);

// For non-class pages
void set_busy(int * descr, bool busy);
void set_pages_in_block(int * descr, int num);

// For class pages
void set_size_of_class(int * descr, int size);
void set_left_blocks(int * descr, int left);
void set_pointer_to_block(int * descr, int * ptr_to_page, void * pointer);


void shift_left(int * what, int n);
void shift_right(int * what, int n);

void set_busy_bytes(int * header, int busy_bytes);
void fill_classes_with_pointers(void * page_ptr, int blocks, int size);
void set_page_free(int num);

int choose_size_of_class(int size);
int find_page_of_class(int class_size);
int find_free_page();
int find_page_of_greater_class(int class_size);

int choose_number_of_pages(int size);
int find_mult_page_block(int pages);
void make_new_block_descs(int block_start, int pages, int size);

void * mem_alloc(size_t size)
{
	// choose size of block
	// find free block
	// change page descriptor

	int search_size = size + sizeof(int);

	// Allocate block in a page
	if (search_size < PAGE_SIZE / 2)
	{
		int class_size = choose_size_of_class(search_size);

		// Find the page of blocks with the class we search for
		int page_of_class = find_page_of_class(class_size);

		// Divide page in blocks, if there were no yet
		if (page_of_class == -1)
		{
			page_of_class = find_free_page();

			if (page_of_class != -1)
			{
				// Make the page of class
				descriptors[page_of_class] = fill_class_descriptor(page_of_class, class_size);
			}
			// No free pages => find the one with the class, which size is closest to ours
			else
			{
				page_of_class = find_page_of_greater_class(class_size);

				// No page, that suits, was found. Wasted
				if (page_of_class == -1)
				{
					return NULL;
				}
			}
		}

		int * descr = &(descriptors[page_of_class]);
		int * ptr_to_page = (int*)global_mem + PAGE_SIZE * page_of_class / sizeof(int);
		int * ptr_to_block = get_pointer_to_block(*descr, ptr_to_page);

		// Point descriptor to next free block
		set_pointer_to_block(descr, ptr_to_page,
			get_pointer_to_block(*ptr_to_block, ptr_to_page));

		// Decrement number of free blocks in the page
		set_left_blocks(descr, get_left_blocks(*descr) - 1);

		// Make header of block
		int header = make_class_header(get_size_of_class(*descr), search_size, page_of_class);

		// Write header
		*ptr_to_block = header;
		return ptr_to_block;
	}

	// Allocate block of pages
	int num_of_pages = choose_number_of_pages(search_size);

	// Find block of free pages
	int block_start = find_mult_page_block(num_of_pages);

	if (block_start == -1)
	{
		return NULL;
	}

	make_new_block_descs(block_start, num_of_pages, search_size);

	return (void *)((int *)global_mem + block_start * PAGE_SIZE / sizeof(int));
}

void * mem_realloc(void * addr, size_t size)
{
	int * ptr_to_block = (int *)addr;

	if (get_head_bit(*ptr_to_block) != 1)
	{
		return NULL;
	}

	int new_size = (int)size + sizeof(int);

		// This is a class block
	if (get_type_class(*ptr_to_block) == true)
	{
		if (new_size == get_busy_bytes(*ptr_to_block))
		{
			return addr;
		}

		if (new_size <= get_size_of_class(*ptr_to_block))
		{
			set_busy_bytes(ptr_to_block, new_size);
			return addr;
		}

		void * new_ptr = mem_alloc(size);
		if (new_ptr != NULL)
		{
			mem_free((void *)ptr_to_block);
			return new_ptr;
		}
		return NULL;
	}

	// This is a non-class block
	if (new_size == (*ptr_to_block) & 0xffff)
	{
		return addr;
	}
	
	int curr_page = (ptr_to_block - global_mem) * sizeof(int) / PAGE_SIZE;
	int pages_in_block = get_pages_in_block(descriptors[curr_page]);
	
	// New size is less, then current block
	if (new_size <= pages_in_block * PAGE_SIZE)
	{
		int new_pages = choose_number_of_pages(new_size);

		// Free blocks, we don't need
		for (int i = new_pages; i < pages_in_block; i++)
		{
			descriptors[curr_page + i] = fill_nonclass_descriptor(curr_page + i, 1, false);
		}

		// Write new length of block
		*ptr_to_block = fill_nonclass_descriptor(0, 0, true) + new_size;

		// Make new page descriptor
		descriptors[curr_page] = fill_nonclass_descriptor(curr_page, new_pages, true);

		return addr;
	}

	// New size is greater, then the block
	void * new_ptr = mem_alloc((size_t)new_size);
	if (new_ptr != NULL)
	{
		mem_free(ptr_to_block);
		return new_ptr;
	}

	return NULL;
}

void mem_free(void * addr)
{
	int * ptr_to_block = (int *)addr;

	if (get_head_bit(*ptr_to_block) != 1)
	{
		return;
	}

	// This is a class block
	if (get_type_class(*ptr_to_block) == true)
	{
		int curr_page = get_page_num(*ptr_to_block);
		int * ptr_to_page = (int*)global_mem + PAGE_SIZE * curr_page / sizeof(int);
		int * descr = &descriptors[curr_page];

		*ptr_to_block = (int)get_pointer_to_block(*descr, ptr_to_page);
		set_pointer_to_block(descr, ptr_to_page, (void *)ptr_to_block);

		// Increment number of free blocks in the page
		set_left_blocks(descr, get_left_blocks(*descr) + 1);

		// Return page to default state - non-class page
		if (get_left_blocks(*descr) == PAGE_SIZE / get_size_of_class(*descr))
		{
			*descr = fill_nonclass_descriptor(curr_page, 1, false);
		}

		return;
	}

	// This is a non-class block
	int curr_page = (ptr_to_block - global_mem) * sizeof(int) / PAGE_SIZE;
	int pages_in_block = get_pages_in_block(descriptors[curr_page]);
	for (int i = 0; i < pages_in_block; i++)
	{
		set_page_free(curr_page + i);
	}
}

int make_descriptor(bool class_type, int page)
{
	int descr = 0;

	// 1 in the first byte of header points to its origin
	set_head_bit(&descr);

	// Set bit pointing at page type
	set_type_class(&descr, class_type);

	// Set number of page
	set_page_num(&descr, page);

	return descr;
}

int fill_class_descriptor(int num, int size)
{
	int descr = make_descriptor(true, num);
	set_size_of_class(&descr, size);
	set_left_blocks(&descr, PAGE_SIZE / size);

	void * page_ptr = (int *)global_mem + num * PAGE_SIZE / sizeof(int);
	fill_classes_with_pointers(page_ptr, PAGE_SIZE / size, size);
	set_pointer_to_block(&descr, (int *)page_ptr, page_ptr);
	return descr;
}

int fill_nonclass_descriptor(int num, int pages_in_block, bool busy)
{
	int descr = make_descriptor(false, num);
	set_busy(&descr, busy);
	set_pages_in_block(&descr, pages_in_block);
	return descr;
}

void set_busy(int * descr, bool busy)
{
	int if_busy = busy;
	shift_left(&if_busy, 22);

	int old_if_busy = (int)get_busy(*descr);
	shift_left(&old_if_busy, 22);

	*descr += if_busy - old_if_busy;
}

bool get_busy(int descr)
{
	shift_right(&descr, 22);
	int busy = descr & 1;
	return true ? busy == 1 : false;
}

void set_pages_in_block(int * descr, int num)
{
	shift_left(&num, 14);

	int old_num = get_pages_in_block(*descr);
	shift_left(&old_num, 14);

	*descr += num - old_num;
}

int get_pages_in_block(int descr)
{
	shift_right(&descr, 14);
	return descr & 255;
}

void set_size_of_class(int * descr, int size)
{
	int pow = -4;
	while (size % 2 == 0)
	{
		pow++;
		size = size / 2;
	}

	shift_left(&pow, 20);
	*descr += pow;
}

int get_size_of_class(int descr)
{
	shift_right(&descr, 20);
	int pow = (descr & 7) + 4;
	int size = 1;

	for (int i = 0; i < pow; i++)
	{
		size *= 2;
	}

	return size;
}

void set_left_blocks(int * descr, int left)
{
	shift_left(&left, 13);

	int old_left = get_left_blocks(*descr);
	shift_left(&old_left, 13);

	*descr += left - old_left;
}

int get_left_blocks(int descr)
{
	shift_right(&descr, 13);
	return descr & 127;
}

void set_pointer_to_block(int * descr, int * ptr_to_page, void * pointer)
{
	int old_descr = *descr;
	shift_right(&old_descr, 7);
	int old_loc_ptr = old_descr & 63;

	//int * page_pointer = (int *)global_mem + PAGE_SIZE * get_page_num(*descr) / sizeof(int);
	int local_pointer = ((int *)pointer - ptr_to_page) * sizeof(int) / 16;

	shift_left(&local_pointer, 7);
	shift_left(&old_loc_ptr, 7);

	*descr += local_pointer - old_loc_ptr;
}

void * get_pointer_to_block(int descr, int * ptr_to_page)
{
	//int * page_pointer = (int *)global_mem + PAGE_SIZE * get_page_num(descr) / sizeof(int);

	shift_right(&descr, 7);
	int local_pointer = descr & 63;

	int * pointer = ptr_to_page + local_pointer * 16 / sizeof(int);
	return (void *)pointer;
}

void set_head_bit(int * value)
{
	int head = 1;
	shift_left(&head, 24);
	*value += head;
}

int get_head_bit(int value)
{
	shift_right(&value, 24);
	return value & 255;
}

void set_page_num(int * descr, int num)
{
	*descr += num;
}

int get_page_num(int descr)
{
	return descr & 127;
}

void set_type_class(int * descr, bool class_type)
{
	int type = class_type;
	shift_left(&type, 23);
	*descr += type;
}

bool get_type_class(int descr)
{
	shift_right(&descr, 23);
	int type_class = descr & 1;
	return true ? type_class == 1 : false;
}

void shift_left(int * what, int n)
{
	*what = *what << n;
}

void shift_right(int * what, int n)
{
	*what = *what >> n;
}

void set_busy_bytes(int * header, int busy_bytes)
{
	busy_bytes -= 1;
	shift_left(&busy_bytes, 11);

	int old_bb = get_busy_bytes(*header) - 1;
	shift_left(&old_bb, 11);

	*header += busy_bytes - old_bb;
}

int get_busy_bytes(int header)
{
	shift_right(&header, 11);
	int busy_bytes = header & 511;
	return busy_bytes + 1;
}

void fill_classes_with_pointers(void * page_ptr, int blocks, int size)
{
	int * this_block = (int *)page_ptr;
	int * next_block;

	for (int i = 0; i < blocks - 1; i++)
	{
		next_block = this_block + size / sizeof(int);
		*this_block = 0;
		set_pointer_to_block(this_block, page_ptr, (void *)next_block);
		this_block = next_block;
	}
	*this_block = 0;
}

void set_page_free(int num)
{
	descriptors[num] = fill_nonclass_descriptor(num, 1, false);
	int * ptr_to_page = (int *)global_mem + PAGE_SIZE * num / sizeof(int);
	*ptr_to_page = 0;
}

int choose_size_of_class(int size)
{
	int class_size = 16;
	while (class_size < size)
	{
		class_size *= 2;
	}
	return class_size;
}

int choose_number_of_pages(int size)
{
	int page_num = PAGE_SIZE;
	while (page_num < size)
	{
		page_num += PAGE_SIZE;
	}
	return page_num / PAGE_SIZE;
}

int find_mult_page_block(int pages)
{
	int block_start = 0;
	while (PAGE_NUM > pages + block_start)
	{
		int descr;

		//int curr_page = block_start;
		for (int i = block_start; i < block_start + pages; i++)
		{
			descr = descriptors[block_start];

			if (get_type_class(descr) != 0 || get_busy(descr) != 0)
			{
				block_start = i + 1;
				break;
			}
		}

		if (get_type_class(descr) == 0 && get_busy(descr) == 0)
		{
			return block_start;
		}
	}
	return -1;
}

void make_new_block_descs(int block_start, int pages, int size)
{
	descriptors[block_start] = fill_nonclass_descriptor(block_start, pages, true);
	int * ptr_to_page = (int *)global_mem + block_start * PAGE_SIZE / sizeof(int);
	*ptr_to_page = fill_nonclass_descriptor(0, 0, true) + size;

	for (int i = 1; i < pages; i++)
	{
		descriptors[block_start + i] = fill_nonclass_descriptor(block_start + i, 0, true);
	}
}

int find_page_of_class(int class_size)
{
	for (int i = 0; i < PAGE_NUM; i++)
	{
		int descr = descriptors[i];
		if (get_type_class(descr) == 1
			&& get_size_of_class(descr) == class_size
			&& get_left_blocks(descr) > 0)
		{
			return i;
		}
	}
	return -1;
}

int find_free_page()
{
	for (int i = 0; i < PAGE_NUM; i++)
	{
		int descr = descriptors[i];
		if (get_type_class(descr) == 0 && get_busy(descr) == 0)
		{
			return i;
		}
	}
	return -1;
}

int find_page_of_greater_class(int class_size)
{
	class_size *= 2;
	while (class_size < PAGE_SIZE)
	{
		int page = find_page_of_class(class_size);
		if (page != -1)
		{
			return page;
		}
	}
	return -1;
}

int make_class_header(int class_size, int busy_bytes, int num)
{
	int header = 0;
	set_head_bit(&header);
	set_type_class(&header, true);
	set_size_of_class(&header, class_size);
	set_busy_bytes(&header, busy_bytes);
	set_page_num(&header, num);

	return header;
}

void init_pages()
{
	// Init all pages as non-class by default
	for (int i = 0; i < PAGE_NUM; i++)
	{
		set_page_free(i);
	}
}
