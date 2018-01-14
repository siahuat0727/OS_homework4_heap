#include <stdlib.h>
#include "hw4_mm_test.h"

void next_chunk_print(struct chunk_header **chunk)
{
	*chunk = find_next_chunk(HEAP, *chunk);
	printf("%p chunk\n", *chunk);
}

#define N_BUFFER 100
int main()
{
	char buf[N_BUFFER];
	while(fgets(buf, N_BUFFER -1, stdin)){
		size_t alloc_bytes;
		void *mem;
		int bin_num;
		if(sscanf(buf, "alloc %lu", &alloc_bytes) == 1)
			printf("0x%08llx\n", (ull)(hw_malloc(alloc_bytes) - (ull)HEAP->start_brk));
		else if(sscanf(buf, "free %p", &mem) == 1)
			printf("%s\n", hw_free(mem) ? "success" : "failed");
		else if(sscanf(buf, "print bin[%d]", &bin_num))
			print_bin(HEAP, bin_num);
		else{
			fprintf(stderr, "can't found command: %s\n", buf);
			exit(EXIT_FAILURE);
		}
	}
}

int maIN()
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


