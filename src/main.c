#include "heap.h"


int main() 
{
	struct Heap* H = create_heap(512, BEST_FIT);

    /*
	char *arr = heap_alloc(H, 10);

	for(int i = 0; i < 10; i++) 
		arr[i] = i;
    */

	test_heap(H);	
}
