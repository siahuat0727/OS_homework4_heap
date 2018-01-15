#include "hw_malloc.h"

#define DEFAULT_HEAP_SIZE (64*kB)
#define MINIMUM_FREE_SPACE (sizeof(struct chunk_header) + 8)

void *hw_malloc(size_t bytes)
{
	to_mult_of_8(&bytes);
	struct chunk_header *chunk = find_free(bytes);
	if(chunk) {
		malloc_chunk(chunk);
		const int required_bytes = bytes + sizeof(struct chunk_header);
		if(chunk->chunk_size >= required_bytes + MINIMUM_FREE_SPACE)
			split(chunk, bytes);
		return relative_addr(HEAP, get_data_ptr(chunk));
	}
	return NULL;
}

int hw_free(void *mem)
{
	// TODO find which heap
	relative_to_real(HEAP, &mem);

	struct chunk_header *chunk = get_chunk_header(mem);
	if(!is_valid(mem) || is_free(chunk))
		return 0;

	free_chunk(chunk);

	struct chunk_header *upper_chunk = find_next_chunk(chunk);
	if (is_free(upper_chunk) && upper_chunk > chunk)
		merge(chunk, upper_chunk);

	struct chunk_header *lower_chunk = find_prev_chunk(chunk);
	if (is_free(lower_chunk) && lower_chunk < chunk)
		merge(lower_chunk, chunk);

	return true;
}

void *get_start_sbrk(void)
{
	return HEAP->start_brk; // TODO multi heap ?
}

void heap_init(struct heap_t** heap, size_t bytes)
{
	*heap = (struct heap_t*)malloc(sizeof(struct heap_t));
	assert(*heap != NULL);
	for (int i = 0; i < BIN_NUM; ++i)
		bin_init(&((*heap)->bin[i]));
	(*heap)->next = NULL;
	(*heap)->size = bytes;
	(*heap)->start_brk = sbrk(bytes);
	assert((*heap)->start_brk != (void*)(-1));

	struct chunk_header *first_chunk = (struct chunk_header*)((*heap)->start_brk);
	chunk_init(first_chunk, bytes, bytes, true); // TODO pre_chunk_size ?
	list_add_decending(first_chunk);
}

void bin_init(struct chunk_header **bin)
{
	*bin = (struct chunk_header*)malloc(sizeof(struct chunk_header));
	assert(*bin != NULL);

	(*bin)->prev = (*bin)->next = *bin;
}

void chunk_init(struct chunk_header *chunk, chunk_size_t chunk_size,
                chunk_size_t pre_chunk_size, chunk_flag_t prev_free_flag)
{
	assert(chunk != NULL);

	chunk->prev = chunk->next = NULL;
	chunk->chunk_size = chunk_size;
	chunk->pre_chunk_size = pre_chunk_size;
	chunk->prev_free_flag = prev_free_flag;
}

void split(struct chunk_header *ori_chunk, size_t split_size)
{
	const int split_chunk_size = sizeof(struct chunk_header) + split_size;
	struct chunk_header* upper_chunk = (struct chunk_header*)((
	                                       void*)ori_chunk + split_chunk_size);

	chunk_size_t pre_chunk_size = ori_chunk->pre_chunk_size;
	chunk_flag_t prev_free_flag = ori_chunk->prev_free_flag;

	if(ori_chunk == find_next_chunk(ori_chunk)) {
		pre_chunk_size = upper_chunk->chunk_size;
		prev_free_flag = true;
	}

	chunk_init(upper_chunk, ori_chunk->chunk_size - split_chunk_size,
	           split_chunk_size, false);
	chunk_init(ori_chunk, split_chunk_size, pre_chunk_size, prev_free_flag);

	assert(upper_chunk->chunk_size >= MINIMUM_FREE_SPACE);
	free_chunk(upper_chunk);
}

void merge(struct chunk_header *lower_chunk, struct chunk_header *upper_chunk)
{
	malloc_chunk(lower_chunk);
	malloc_chunk(upper_chunk);
	chunk_init(lower_chunk, lower_chunk->chunk_size + upper_chunk->chunk_size,
	           lower_chunk->pre_chunk_size, lower_chunk->prev_free_flag);
	free_chunk(lower_chunk);
}

struct heap_t* get_heap(const struct chunk_header *chunk)
{
	for (struct heap_t *iter = HEAP; iter != NULL; iter = iter->next)
		if(inside_heap(iter, chunk))
			return iter;
	return NULL;
}

int get_bin_num(size_t bytes)
{
	assert((bytes & 0b111) == 0 && bytes > 0);
	const int index = (bytes >> 3) - 1;
	return index < BIN_NUM - 1 ? index : BIN_NUM - 1;
}

void* get_data_ptr(const struct chunk_header const *chunk)
{
	return (void*)chunk + sizeof(struct chunk_header);
}

struct chunk_header* get_chunk_header(const void const *data)
{
	return (struct chunk_header*)(data - sizeof(struct chunk_header));
}

size_t get_data_size(const struct chunk_header const *chunk)
{
	return chunk->chunk_size - sizeof(struct chunk_header);
}

void __list_add(struct chunk_header *new_lst, struct chunk_header *prev,
                struct chunk_header *next)
{
	assert(new_lst != NULL && new_lst->next == NULL && new_lst->prev == NULL);
	assert(next != NULL && prev != NULL);

	next->prev = new_lst;
	new_lst->next = next;
	new_lst->prev = prev;
	prev->next = new_lst;
}

void __list_del(struct chunk_header *prev, struct chunk_header *next)
{
	assert(next != NULL && prev != NULL);
	next->prev = prev;
	prev->next = next;
}

void list_add_decending(struct chunk_header *chunk)
{
	assert(chunk != NULL && chunk->next == NULL && chunk->prev == NULL);

	struct heap_t *heap = get_heap(chunk);
	struct chunk_header *bin = heap->bin[get_bin_num(get_data_size(chunk))];
	struct chunk_header *iter = bin->next;
	while (iter != bin) {
		if (chunk->chunk_size > iter->chunk_size)
			break;
		iter = iter->next;
	}
	__list_add(chunk, iter->prev, iter);
}

void list_del(struct chunk_header *chunk)
{
	// if valid, then
	assert(chunk != NULL);
	__list_del(chunk->prev, chunk->next);
	chunk->prev = NULL;
	chunk->next = NULL;
}

bool inside_heap(const struct heap_t const *heap,
                 const struct chunk_header *entry)
{
	return (void*)entry >= heap->start_brk
	       && (void*)entry < heap->start_brk + heap->size;
}

struct chunk_header* find_prev_chunk(const struct chunk_header *chunk)
{
	struct chunk_header *prev_chunk = (struct chunk_header*)((
	                                      void*)chunk - chunk->pre_chunk_size);
	struct heap_t *heap = get_heap(chunk);
	if(!inside_heap(heap, prev_chunk)) {
		prev_chunk = (struct chunk_header*)((void*)prev_chunk + heap->size);
		assert((void*)prev_chunk + prev_chunk->chunk_size == heap->start_brk +
		       heap->size);
	}
	return prev_chunk;
}

struct chunk_header* find_next_chunk(const struct chunk_header *chunk)
{
	struct chunk_header *next_chunk = (struct chunk_header*)((
	                                      void*)chunk + chunk->chunk_size);
	struct heap_t *heap = get_heap(chunk);
	if(!inside_heap(heap, next_chunk)) {
		next_chunk = (struct chunk_header*)((void*)next_chunk - heap->size);
		assert((void*)next_chunk == heap->start_brk);
	}
	return next_chunk;
}

bool is_free(const struct chunk_header const *chunk)
{
	struct chunk_header *next_chunk = find_next_chunk(chunk);
	return next_chunk->prev_free_flag;
}

bool is_empty(const struct chunk_header const *bin)
{
	return bin->next == bin;
}

bool is_valid(void *mem)
{
	struct chunk_header *chunk = get_chunk_header(mem);
	struct heap_t *heap = get_heap(chunk);
	struct chunk_header *first_chunk = (struct chunk_header *)(heap->start_brk);
	if(chunk == first_chunk)
		return true;
	for(struct chunk_header *iter_chunk = find_next_chunk(first_chunk);
	    iter_chunk != first_chunk; iter_chunk = find_next_chunk(iter_chunk)) {
		if(iter_chunk == chunk)
			return true;
	}
	return false;
}

struct chunk_header *try_find_free_bin(const struct chunk_header const *bin,
                                       size_t bytes)
{
	assert(is_empty(bin) == false);
	const int min_size = bytes + sizeof(struct chunk_header);
	struct chunk_header *iter = bin->prev;
	while (iter != bin) {
		if (iter->chunk_size >= min_size)
			break;
		iter = iter->prev;
	}
	if(iter == bin)
		return NULL;
	while(iter != bin) {
		if(iter->prev->chunk_size > iter->chunk_size || iter->prev == bin)
			return iter;
		iter = iter->prev;
	}
	assert(iter != bin);
	return NULL;
}

struct chunk_header *find_free(size_t bytes)
{
	struct heap_t **heap = &HEAP;
	while(true) {
		if(*heap == NULL)
			heap_init(heap, DEFAULT_HEAP_SIZE);
		assert(*heap != NULL);
		int bin_start = get_bin_num(bytes);
		for(int i = bin_start; i < BIN_NUM; ++i) {
			if(!is_empty((*heap)->bin[i])) {
				struct chunk_header *free_chunk = try_find_free_bin((*heap)->bin[i], bytes);
				if(free_chunk != NULL)
					return free_chunk;
			}
		}
		heap = &((*heap)->next);
	}
}

void malloc_chunk(struct chunk_header *chunk)
{
	struct chunk_header *next_chunk = find_next_chunk(chunk);
	next_chunk->prev_free_flag = false;
	list_del(chunk); // TODO try?
}

void free_chunk(struct chunk_header *chunk)
{
	struct chunk_header *next_chunk = find_next_chunk(chunk);
	next_chunk->prev_free_flag = true;
	next_chunk->pre_chunk_size = chunk->chunk_size;
	list_add_decending(chunk);
}

void print_relative_addr(const struct heap_t const *heap,
                         struct chunk_header *chunk)
{
	printf("0x%08llx", (ull)relative_addr(heap, chunk));
}

void relative_to_real(const struct heap_t const *heap, void **mem)
{
	*mem += (ull)heap->start_brk;
}

void *relative_addr(const struct heap_t const *heap, struct chunk_header *chunk)
{
	return (void*)chunk - (ull)(heap->start_brk);
}

void to_mult_of_8(size_t* bytes)
{
	*bytes += (((*bytes & 0b111) ^ 0b111) + 1) & 0b111;
}

void print_bin(const struct heap_t const *heap, int i)
{
	struct chunk_header *bin = heap->bin[i];
	for(struct chunk_header *iter = bin->next; iter != bin; iter = iter->next)
		printf("0x%08llx--------%llu\n", (ull)relative_addr(heap, iter),
		       iter->chunk_size);
}
