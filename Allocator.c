#include "Allocator.h"

#include <math.h>

int make_descriptor(bool class_type, int num);

int fill_class_descriptor(int num, int size);

int fill_nonclass_descriptor(int num, int next_pages);

// For non-class pages
void set_busy(int * descr, bool busy);
int get_busy(int descr);

void set_pages_in_block(int * descr, int num);
int get_pages_in_block(int descr);

// For class pages
void set_size_of_class(int * descr, int size);
int get_size_of_class(int descr);

void set_left_blocks(int * descr, int left);
int get_left_blocks(int descr);

void set_pointer_to_block(int * descr, void * pointer);
void * get_pointer_to_block(int descr);


void set_page_num(int * descr, int num);
int get_page_num(int descr);

void set_type(int * descr, bool class_type);
int get_type(int descr);


void shift_left(int * what, int n);
void shift_right(int * what, int n);


void fill_classes_with_pointers(void * page_ptr, int blocks, int size);


void * mem_alloc(size_t size)
{




	return NULL;
}

void * mem_realloc(void * addr, size_t size)
{
	return NULL;
}

void mem_free(void * addr)
{
}

int make_descriptor(bool class_type, int num)
{
	int header = 1;

	// 1 in the first byte of header points to its origin
	shift_left(&header, 24);
	int descr = header;

	// Set bit pointing at page type
	set_type(&descr, class_type);

	// Set number of page
	set_page_num(&descr, num);

	return descr;
}

int fill_class_descriptor(int num, int size)
{
	int descr = make_descriptor(true, num);
	set_size_of_class(&descr, size);
	set_left_blocks(&descr, PAGE_SIZE / size);

	void * page_ptr = (int *)global_mem + num * PAGE_SIZE / sizeof(int);
	fill_classes_with_pointers(page_ptr, PAGE_SIZE / size, size);
	set_pointer_to_block(&descr, page_ptr);
	return descr;
}

int fill_nonclass_descriptor(int num, int next_pages)
{
	int descr = make_descriptor(false, num);
	set_busy(&descr, false);
	set_pages_in_block(&descr, next_pages);
	return descr;
}

void set_busy(int * descr, bool busy)
{
	int if_busy = busy;
	shift_left(&if_busy, 22);
	*descr += if_busy;
}

int get_busy(int descr)
{
	shift_right(&descr, 22);
	return descr & 1;
}

void set_pages_in_block(int * descr, int num)
{
	shift_left(&num, 15);
	*descr += num;
}

int get_pages_in_block(int descr)
{
	shift_right(&descr, 15);
	return descr & 127;
}

void set_size_of_class(int * descr, int size)
{
	shift_left(&size, 20);
	*descr += size;
}

int get_size_of_class(int descr)
{
	shift_right(&descr, 20);
	return descr & 7;
}

void set_left_blocks(int * descr, int left)
{
	shift_left(&left, 13);
	*descr += left;
}

int get_left_blocks(int descr)
{
	shift_right(&descr, 13);
	return descr & 127;
}

void set_pointer_to_block(int * descr, void * pointer)
{
	int * page_pointer = (int *)global_mem + PAGE_SIZE * get_page_num(*descr) / sizeof(int);
	int local_pointer = ((int *)pointer - page_pointer) * sizeof(int) / 16;

	shift_left(&local_pointer, 7);
	*descr += local_pointer;
}

void * get_pointer_to_block(int descr)
{
	int * page_pointer = (int *)global_mem + PAGE_SIZE * get_page_num(descr) / sizeof(int);

	shift_right(&descr, 7);
	int local_pointer = descr & 63;

	int * pointer = page_pointer + local_pointer * 16 / sizeof(int);
	return (void *)pointer;
}

void set_page_num(int * descr, int num)
{
	*descr += num;
}

int get_page_num(int descr)
{
	return descr & 127;
}

void set_type(int * descr, bool class_type)
{
	int type = class_type;
	shift_left(&type, 23);
	*descr += type;
}

int get_type(int descr)
{
	shift_right(&descr, 23);
	return descr & 1;
}

void shift_left(int * what, int n)
{
	*what = *what << n;
}

void shift_right(int * what, int n)
{
	*what = *what >> n;
}

void fill_classes_with_pointers(void * page_ptr, int blocks, int size)
{
	int * this_block = (int *)page_ptr;

	for (int i = 0; i < blocks - 1; i++)
	{
		*this_block = (int)this_block + size;
		this_block = (int *)*this_block;
	}
	*this_block = (int)NULL;
}

void init_pages()
{
	int * page_ptr = global_mem;
	for (int i = 0; i < PAGE_NUM; i++)
	{
		*page_ptr = fill_nonclass_descriptor(i, 0);
		page_ptr += PAGE_SIZE / sizeof(int);
	}
}
