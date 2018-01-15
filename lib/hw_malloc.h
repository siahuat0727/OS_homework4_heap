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
	struct heap_t *next;
	size_t size;
};

struct heap_t *HEAP;

extern void *hw_malloc(size_t bytes);
extern int hw_free(void *mem);
extern void *get_start_sbrk(void);

// init
void heap_init(struct heap_t** heap, size_t bytes);
void bin_init(struct chunk_header **bin);
void chunk_init(struct chunk_header *chunk, chunk_size_t chunk_size,
                chunk_size_t pre_chunk_size, chunk_flag_t prev_free_flag);

// split and merge
void split(struct chunk_header *ori_chunk, size_t split_size);
void merge(struct chunk_header *lower_chunk, struct chunk_header *upper_chunk);

// get sth
struct heap_t* get_heap(const struct chunk_header *chunk);
int get_bin_num(size_t bytes);
void* get_data_ptr(const struct chunk_header const *chunk);
struct chunk_header* get_chunk_header(const void const *data);
size_t get_data_size(const struct chunk_header const *chunk);

// list
void __list_add(struct chunk_header *new_lst, struct chunk_header *prev,
                struct chunk_header *next);
void __list_del(struct chunk_header *prev, struct chunk_header *next);
void list_add_decending(struct chunk_header *entry);
void list_del(struct chunk_header *entry);

// heap
bool inside_heap(const struct heap_t const *heap,
                 const struct chunk_header *entry);
struct chunk_header* find_prev_chunk(const struct chunk_header *entry);
struct chunk_header* find_next_chunk(const struct chunk_header *entry);

// check
bool is_free(const struct chunk_header const *chunk);
bool is_empty(const struct chunk_header const *bin);
bool is_valid(void *mem);


struct chunk_header *try_find_free_bin(const struct chunk_header const *bin,
                                       size_t bytes);
struct chunk_header *find_free(size_t bytes);

void malloc_chunk(struct chunk_header *chunk);
void free_chunk(struct chunk_header *chunk);

void print_relative_addr(const struct heap_t const *heap,
                         struct chunk_header *chunk);
void relative_to_real(const struct heap_t const *heap, void **mem);
void *relative_addr(const struct heap_t const *heap,
                    struct chunk_header *chunk);
void to_mult_of_8(size_t* bytes);
void print_bin(const struct heap_t const *heap, int i);

#endif
