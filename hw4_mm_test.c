#include "hw4_mm_test.h"

void next_chunk_print(struct chunk_header **chunk)
{
	*chunk = find_next_chunk(HEAP, *chunk);
	printf("%p chunk\n", *chunk);
}

int main()
{

	int* a = hw_malloc(8);
	assert(a != NULL);
	printf("%p heap\n", HEAP->start_brk);
	printf("%p heap end\n", HEAP->start_brk + 64*1024);
	puts("malloc a 8");
	print_bin_all();

	struct chunk_header *chunk = get_chunk_header((void*)a);




	int *b = hw_malloc(14);
	assert(b != NULL);
	puts("malloc b  14");
	print_bin_all();

	hw_free(a);
	puts("free a");
	print_bin_all();

	int *c = hw_malloc(20);
	puts("malloc c  20");
	print_bin_all();

	hw_free(b);
	puts("free b");
	print_bin_all();

	int *d = hw_malloc(14);
	assert(d != NULL);
	puts("malloc d  14");
	print_bin_all();





	return 0;
}

int maiN()
{
	printf("chunk_header size: %ld\n", sizeof(struct chunk_header));
	printf("%p\n", hw_malloc(8));
	printf("%s\n", hw_free(NULL) == 1 ? "success" : "fail");
	printf("start_brk: %p\n", get_start_sbrk());
	return 0;
}


