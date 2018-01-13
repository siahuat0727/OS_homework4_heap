#ifndef HW_MALLOC_H
#define HW_MALLOC_H

#include <stddef.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define kB (1ULL << 10)
#define BIN_NUM 7

typedef unsigned long long ull;
typedef struct chunk_header* chunk_ptr_t;
typedef ull chunk_size_t;
typedef ull chunk_flag_t;

struct chunk_header {
	chunk_ptr_t prev;
	chunk_ptr_t next;
	chunk_size_t chunk_size;
	chunk_size_t pre_chunk_size;
	chunk_flag_t prev_free_flag;
};

struct heap_t {
	struct chunk_header *bin[BIN_NUM];
	void* start_brk;
	struct heap_t* next;
	size_t size;
};

struct heap_t *HEAP;

extern void *hw_malloc(size_t bytes);
extern int hw_free(void *mem);
extern void *get_start_sbrk(void);

void heap_init(struct heap_t* heap, size_t bytes);
void bin_init(struct chunk_header* bin);

void __list_add(struct chunk_header *new_lst, struct chunk_header *prev,
                struct chunk_header *next);
void __list_del(struct chunk_header *prev, struct chunk_header *next);
void list_add_decending(struct chunk_header *head, struct chunk_header *entry);
bool list_try_del(struct chunk_header *entry);


void split_upper_chunk_init(struct chunk_header* chunk,
                            struct chunk_header *prev_chunk, size_t bytes);
void* first_split(struct chunk_header* split_chunk, size_t total_size,
                  size_t split_size);


#endif
