#pragma once

#define PAGE_SIZE 1024
#define PAGE_NUM 6
#define MEM_SIZE PAGE_SIZE * PAGE_NUM

#include <stdio.h>

//typedef int size_t;
typedef enum { false, true } bool;

void * global_mem;

void * mem_alloc(size_t size);

void * mem_realloc(void *addr, size_t size);

void mem_free(void *addr);

void init_pages();
