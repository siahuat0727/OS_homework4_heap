#include "hw_malloc.h"

#define DEFAULT_HEAP_SIZE (4*kB)

void *hw_malloc(size_t bytes)
{
	static bool first_call = true;
	if(first_call) {
		first_call = false;
		heap_init(HEAP, DEFAULT_HEAP_SIZE);
		puts("before call first_split");
		fflush(stdout);
		return first_split((struct chunk_header*)(HEAP->start_brk), DEFAULT_HEAP_SIZE,
		                   bytes);
	}
	return NULL;
}

int hw_free(void *mem)
{
	return 0;
}

void *get_start_sbrk(void)
{
	return NULL;
}



void heap_init(struct heap_t* heap, size_t bytes)
{
	heap = (struct heap_t*)malloc(sizeof(struct heap_t));
	assert(heap != NULL);
	for (int i = 0; i < BIN_NUM; ++i) {
		bin_init(heap->bin[i]);
	}
	heap->next = NULL;
	heap->size = bytes;
	heap->start_brk = sbrk(bytes);
	assert((void*)(heap->start_brk) != (void*)(-1));
	printf("size of heap = %lu, %llu\n", heap->size,
	       (ull)sbrk(0) - (ull)heap->start_brk);
}

void bin_init(struct chunk_header* bin)
{
	bin = (struct chunk_header*)malloc(sizeof(struct chunk_header));
	assert(bin != NULL);

	bin->prev = bin->next = bin;
	bin->chunk_size = sizeof(struct chunk_header);
	bin->pre_chunk_size = bin->chunk_size;
	bin->prev_free_flag = true;
}

void split_upper_chunk_init(struct chunk_header* chunk,
                            struct chunk_header *prev_chunk, size_t bytes)
{
	assert(chunk != NULL && prev_chunk != NULL);

	chunk->prev = prev_chunk;
	chunk->next = prev_chunk->next;
	chunk->chunk_size = bytes;
	chunk->pre_chunk_size = prev_chunk->chunk_size;
	chunk->prev_free_flag = false;
}

void* first_split(struct chunk_header* split_chunk, size_t total_size,
                  size_t split_size)
{
	puts("0");
	fflush(stdout);
	struct chunk_header* upper_chunk = (struct chunk_header*)((
	                                       ull)split_chunk + split_size);
	split_upper_chunk_init(upper_chunk, split_chunk, total_size - split_size);
	puts("1");
	fflush(stdout);

	split_chunk->chunk_size = split_size;
	split_chunk->next = upper_chunk;
	split_chunk->prev = upper_chunk;
	split_chunk->pre_chunk_size = upper_chunk->chunk_size;
	split_chunk->prev_free_flag = true;
	return (void*)((ull)split_chunk + sizeof(struct chunk_header));

	return NULL;

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

void list_add_decending(struct chunk_header *head, struct chunk_header *entry)
{
	assert(head != NULL && head->next != NULL && head->prev != NULL);
	assert(entry != NULL && entry->next == NULL && entry->prev == NULL);

	struct chunk_header *iter = head->next;
	while (iter != head) {
		if (entry->chunk_size < iter->chunk_size)
			break;
		iter = iter->next;
	}
	__list_add(entry, iter->prev, iter);
}

bool list_try_del(struct chunk_header *entry)
{
	// if valid, then
	assert(entry != NULL);
	__list_del(entry->prev, entry->next);
	return true;
}




