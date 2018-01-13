#include "hw4_mm_test.h"


int main()
{

	int* a = hw_malloc(4);
	puts("after hw_malloc"); fflush(stdout);
	printf("%p int\n", a);
	printf("%p heap\n", HEAP->start_brk + 40);
	return 0;	
}

int maiN() {
    printf("chunk_header size: %ld\n", sizeof(struct chunk_header));
    printf("%p\n", hw_malloc(8));
    printf("%s\n", hw_free(NULL) == 1 ? "success" : "fail");
    printf("start_brk: %p\n", get_start_sbrk());
    return 0;
}
