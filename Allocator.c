#include "Allocator.h"

#include <math.h>

int make_descriptor(bool class_type, int num);
int make_class_header(int class_size, int busy_bytes);

int fill_class_descriptor(int num, int size);

int fill_nonclass_descriptor(int num, int pages_in_block);


void set_head_bit(int * value);
void set_page_num(int * descr, int num);
void set_type_class(int * descr, bool class_type);

// For non-class pages
void set_busy(int * descr, bool busy);
void set_pages_in_block(int * descr, int num);

// For class pages
void set_size_of_class(int * descr, int size);
void set_left_blocks(int * descr, int left);
void set_pointer_to_block(int * descr, void * pointer);


void shift_left(int * what, int n);
void shift_right(int * what, int n);

void set_busy_bytes(int * header, int busy_bytes);

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

//********************************************************************************************
int make_descriptor(bool class_type, int num)
{
	int descr = 0;

	// 1 in the first byte of header points to its origin
	set_head_bit(&descr);

	// Set bit pointing at page type
	set_type_class(&descr, class_type);

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

//***************************************************************************************************
int fill_nonclass_descriptor(int num, int pages_in_block)
{
	int descr = make_descriptor(false, num);
	set_busy(&descr, false);
	set_pages_in_block(&descr, pages_in_block);
	return descr;
}

void set_busy(int * descr, bool busy)
{
	int if_busy = busy;
	shift_left(&if_busy, 22);
	*descr += if_busy;
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
	*descr += num;
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
	int pow = descr & 7 + 4;
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

void set_head_bit(int * value)
{
	int head = 1;
	shift_left(&head, 24);
	*value += head;
}

int get_head_bit(int value)
{
	shift_left(&value, 24);
	return value & 8;
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
	int free_bytes = get_size_of_class(*header) - busy_bytes;
	shift_left(&free_bytes, 17);
	*header += free_bytes;
}

int get_busy_bytes(int header)
{
	int size_of_class = get_size_of_class(header);
	shift_right(&header, 17);
	int free_bytes = header & 4;
	return size_of_class - free_bytes;
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

int make_class_header(int class_size, int busy_bytes)
{
	int header = 0;
	set_head_bit(&header);
	set_size_of_class(&header, class_size);
	set_busy_bytes(&header, busy_bytes);

	return header;
}

void init_pages()
{
	// Init all pages as non-class by default
	for (int i = 0; i < PAGE_NUM; i++)
	{
		descriptors[i] = fill_nonclass_descriptor(i, 1);
	}
}
