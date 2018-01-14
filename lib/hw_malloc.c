#include "hw_malloc.h"

#define DEFAULT_HEAP_SIZE (64*kB)
#define MINIMUM_FREE_SPACE (sizeof(struct chunk_header) + 8)

#define my_printf(str, arg) do{						\
								printf(str, arg);	\
								fflush(stdout);		\
							}while(0)


void my_puts(const char *str)
{
	puts(str);
	fflush(stdout);
}

void *hw_malloc(size_t bytes)
{
	to_mult_of_8(&bytes);
	if(HEAP == NULL)
		heap_init(&HEAP, DEFAULT_HEAP_SIZE);
	struct chunk_header *chunk = try_find_free(HEAP, bytes);
	if(chunk){
		printf("%llu found chunk_size in hw_malloc\n", chunk->chunk_size);
		malloc_chunk(chunk);
		const int required_bytes = bytes + sizeof(struct chunk_header);
		if(chunk->chunk_size >= required_bytes + MINIMUM_FREE_SPACE)
			split(chunk, bytes);
		return get_data_ptr(chunk);
	}
	return NULL;
}

int hw_free(void *mem)
{
	// TODO find which heap
	// check if valid
	struct chunk_header *chunk = get_chunk_header(mem);
	free_chunk(chunk);

	return 0;
}

void *get_start_sbrk(void)
{
	return HEAP->start_brk; // TODO multi heap ?
}

void heap_init(struct heap_t** heap, size_t bytes)
{
	*heap = (struct heap_t*)malloc(sizeof(struct heap_t));
	assert(*heap != NULL);
	for (int i = 0; i < BIN_NUM; ++i) {
		bin_init(&((*heap)->bin[i]));
	}
	(*heap)->next = NULL;
	(*heap)->size = bytes;
	(*heap)->start_brk = sbrk(bytes);
	assert((*heap)->start_brk != (void*)(-1));
	
	struct chunk_header *first_chunk = (struct chunk_header*)((*heap)->start_brk);
	chunk_init(first_chunk, bytes, bytes, true); // TODO pre_chunk_size ?
	list_add_decending(HEAP, first_chunk);
}

void bin_init(struct chunk_header **bin)
{
	*bin = (struct chunk_header*)malloc(sizeof(struct chunk_header));
	assert(*bin != NULL);

	(*bin)->prev = (*bin)->next = *bin;
	(*bin)->chunk_size = sizeof(struct chunk_header);
	(*bin)->pre_chunk_size = (*bin)->chunk_size;
	(*bin)->prev_free_flag = true;
}

void chunk_init(struct chunk_header *chunk, chunk_size_t chunk_size, chunk_size_t pre_chunk_size, chunk_flag_t prev_free_flag)
{
	assert(chunk != NULL);

	chunk->prev = NULL;
	chunk->next = NULL;
	chunk->chunk_size = chunk_size;
	chunk->pre_chunk_size = pre_chunk_size;
	chunk->prev_free_flag = prev_free_flag;
}

bool split(struct chunk_header *ori_chunk, size_t split_size) // TODO smaller than 48
{
	my_puts("split");
	const int split_chunk_size = sizeof(struct chunk_header) + split_size;
	struct chunk_header* upper_chunk = (struct chunk_header*)((void*)ori_chunk + split_chunk_size);

	chunk_size_t pre_chunk_size = ori_chunk->pre_chunk_size;
	chunk_flag_t prev_free_flag = ori_chunk->prev_free_flag;

	if(ori_chunk->chunk_size == DEFAULT_HEAP_SIZE){ // only one chunk // TODO change to one
		assert(ori_chunk == find_next_chunk(HEAP, ori_chunk));
		pre_chunk_size = upper_chunk->chunk_size;
		prev_free_flag = true;
	}

	chunk_init(upper_chunk, ori_chunk->chunk_size - split_chunk_size, split_chunk_size, false);
	chunk_init(ori_chunk, split_chunk_size, pre_chunk_size, prev_free_flag);
	
	free_chunk(upper_chunk);

	return true;
}

int get_bin_num(size_t bytes)
{
	assert((bytes & 0b111) == 0);
	assert(bytes > 0);
	printf("bytes = %u\n", bytes); fflush(stdout);
	const int index = (bytes >> 3) - 1;
	printf("index = %u\n", index); fflush(stdout);
   	return index < BIN_NUM - 1 ? index : BIN_NUM - 1;	
}

void* get_data_ptr(struct chunk_header *chunk)
{
	return (void*)chunk + sizeof(struct chunk_header);
}

struct chunk_header* get_chunk_header(void *data)
{
	return (struct chunk_header*)(data - sizeof(struct chunk_header));
}

size_t get_data_size(struct chunk_header *chunk)
{
	return chunk->chunk_size - sizeof(struct chunk_header);
}

void __list_add(struct chunk_header *new_lst, struct chunk_header *prev, struct chunk_header *next)
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

void list_add_decending(const struct heap_t const *heap, struct chunk_header *entry)
{
	my_puts("list_add_decending");
	assert(heap != NULL);
	assert(entry != NULL && entry->next == NULL && entry->prev == NULL);

	my_puts("1");
	printf("entry size = %llu\n", entry->chunk_size);
	struct chunk_header *bin = heap->bin[get_bin_num(get_data_size(entry))];
	my_puts("2");
	struct chunk_header *iter = bin->next;
	while (iter != bin) {
		my_puts("while");
		if (entry->chunk_size > iter->chunk_size)
			break;
		iter = iter->next;
	}
	__list_add(entry, iter->prev, iter);
	my_puts("list_add_decending end");
}

bool list_try_del(struct chunk_header *entry)
{
	// if valid, then
	assert(entry != NULL);
	__list_del(entry->prev, entry->next);
	entry->prev = NULL;
	entry->next = NULL;

	return true;
}

bool inside_heap(const struct heap_t const *heap, struct chunk_header *entry)
{
	return (void*)entry >= heap->start_brk && (void*)entry < heap->start_brk + heap->size;
}

struct chunk_header* find_next_chunk(const struct heap_t const *heap, struct chunk_header *entry)
{

	struct chunk_header *next_chunk = (struct chunk_header*)((void*)entry + entry->chunk_size);
	if(!inside_heap(heap, next_chunk)){
		next_chunk = (struct chunk_header*)((void*)next_chunk - heap->size);
		assert((void*)next_chunk == heap->start_brk);
	}
	return next_chunk;
}

bool empty(const struct chunk_header const *bin)
{
	return bin->next == bin;
}

struct chunk_header *try_find_free_bin(const struct chunk_header const *bin, size_t bytes)
{
	assert(empty(bin) == false);
	const int min_size = bytes + sizeof(struct chunk_header);
	struct chunk_header *iter = bin->next;
	while (iter != bin) {
		if (iter->chunk_size == min_size)
			return iter;
		else if(iter->chunk_size < min_size)
			break;
		iter = iter->next;
	}
	if(iter->prev == bin)
		return NULL;
	return iter->prev;
}

struct chunk_header *try_find_free(const struct heap_t const *heap, size_t bytes)
{
	int bin_start = get_bin_num(bytes);
	for(int i = bin_start; i < BIN_NUM; ++i){
		if(!empty(heap->bin[i])){
			return try_find_free_bin(heap->bin[i], bytes);
		}
	}
	return NULL; // TODO malloc new heap
}

void malloc_chunk(struct chunk_header *chunk)
{
	struct chunk_header *next_chunk = find_next_chunk(HEAP, chunk);
	next_chunk->prev_free_flag = false;
	list_try_del(chunk); // TODO try?
}

void free_chunk(struct chunk_header *chunk)
{
	my_puts("free_chunk");
	struct chunk_header *next_chunk = find_next_chunk(HEAP, chunk);
	next_chunk->prev_free_flag = true;
	next_chunk->pre_chunk_size = chunk->chunk_size;
	list_add_decending(HEAP, chunk); // TODO try?
	my_puts("free_chunk end");
}

ull relative_addr(const struct heap_t const *heap, struct chunk_header *entry)
{
	return (ull)entry - (ull)(heap->start_brk);
}

void to_mult_of_8(size_t* bytes)
{
	*bytes += (((*bytes & 0b111) ^ 0b111) + 1) & 0b111;
}

void print_bin(const struct heap_t const *heap, int i)
{
	printf("bin %d: \n", i);
	struct chunk_header *bin = heap->bin[i];
	for(struct chunk_header *iter = bin->next; iter != bin; iter = iter->next){
		printf("0x%08llx--------%llu\n", relative_addr(heap, iter), iter->chunk_size);
	}
	puts("");
}

void print_bin_all()
{
	for (int i = 0; i < BIN_NUM; ++i)
		print_bin(HEAP, i);
}
