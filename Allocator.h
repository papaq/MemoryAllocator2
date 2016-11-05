#pragma once

#define PAGE_SIZE 1024
#define PAGE_NUM 6
#define MEM_SIZE PAGE_SIZE * PAGE_NUM

#include <stdio.h>

typedef enum { false, true } bool;

// Pointer to start of the allocated space
void * global_mem;

// Array of pages' descriptors
int descriptors[PAGE_NUM];

void * mem_alloc(size_t size);

void * mem_realloc(void *addr, size_t size);

void mem_free(void *addr);

void init_pages();

// Get 31-24 bits of value
int get_head_bit(int value);

// Get type of the page
bool get_type_class(int descr);

// Get number of the page
int get_page_num(int descr);

// Get the page status
bool get_busy(int descr);

// Get number of pages, that stand the block
int get_pages_in_block(int descr);

// Get size of class
int get_size_of_class(int descr);

// Get number of left free blocks
int get_left_blocks(int descr);

// Get pointer to first free block
void * get_pointer_to_block(int descr);

// Get busy bytes of the class
int get_busy_bytes(int header);
