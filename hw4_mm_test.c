#include <stdlib.h>
#include "hw4_mm_test.h"

#define N_BUFFER 100
int main()
{
	char buf[N_BUFFER];
	while(fgets(buf, N_BUFFER -1, stdin)) {
		size_t alloc_bytes;
		void *mem;
		int bin_num;
		if(sscanf(buf, "alloc %lu", &alloc_bytes) == 1)
			printf("0x%08llx\n", (ull)hw_malloc(alloc_bytes));
		else if(sscanf(buf, "free %p", &mem) == 1)
			printf("%s\n", hw_free(mem) ? "success" : "fail");
		else if(sscanf(buf, "print bin[%d]", &bin_num) == 1)
			print_bin(HEAP, bin_num);
		else {
			fprintf(stderr, "can't found command: %s\n", buf);
			exit(EXIT_FAILURE);
		}
	}
}

int maiN()
{
	printf("chunk_header size: %ld\n", sizeof(struct chunk_header));
	printf("%p\n", hw_malloc(8));
	printf("%s\n", hw_free(NULL) == 1 ? "success" : "fail");
	printf("start_brk: %p\n", get_start_sbrk());
	return 0;
}


