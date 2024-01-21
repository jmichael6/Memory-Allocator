#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define WORD_SIZE 4     //word size in bits
#define BLOCK_ALIGN 1   //is specified in words
					   
typedef char BYTE;

#if WORD_SIZE == 8
typedef long long WORD;
#define WFX "llx"  
#define WFS "lld"  

#elif WORD_SIZE == 4
typedef int WORD;
#define WFX "x"
#define WFS "d"

#endif


// Defining the heap 

typedef enum {FIRST_FIT, BEST_FIT, NEXT_FIT} POLICIES; 

struct Heap {
 	WORD size; 
	BYTE *bytes;

	POLICIES policy;
	WORD* next_free;
	WORD* head_free;
};


struct Heap* create_heap(int size, POLICIES policy);


char* heap_alloc(struct Heap* H, int bytes);
void heap_free(struct Heap* H, char* payload);

void test_heap(struct Heap* H);
