#include "heap.h"

//Functions for the Heap

void init_heap(struct Heap* H);

struct Heap* create_heap(int size, POLICIES policy) { 	
	if(size <= 0 || size % (WORD_SIZE * BLOCK_ALIGN) != 0) {
		printf("ERROR: Invalid size passed to create_heap()\n");
		exit(1);
	} else {
		size /= WORD_SIZE;  //Converting from bytes to words		

		struct Heap *H = malloc(sizeof(struct Heap));
		H->size = size;
		H->bytes = malloc(sizeof(WORD) * size);
		H->next_free = NULL;
		H->policy = policy;
		H->head_free = NULL;

		init_heap(H);
		return H;
	}
}





//Selector functions

WORD get_header(WORD* block) {
	return *block;
}

int get_a_bit(WORD* block) {
	return get_header(block) & 0x1;
}

int is_free(WORD* block) {
	return get_a_bit(block) == 0;
}

WORD size_from_header(WORD header) {
	return (header & ~(BLOCK_ALIGN * WORD_SIZE - 1)) / WORD_SIZE;	
	//return header & ~(BLOCK_ALIGN - 1);	
}

WORD get_size(WORD* block) {
	return size_from_header(get_header(block));
	//return get_header(block) & ~(BLOCK_ALIGN - 1);
}





// Setter functions

void set_header(WORD* block, WORD size, int a) {
	*block = size * WORD_SIZE | a;	
	//*block = size | a;	
}

void set_a_bit(WORD* block, int a) ;



// An implementation of merging and splitting

void create_block(WORD* block, WORD size);

void merge_blocks(WORD* lblock, WORD* rblock) {
	WORD size = get_size(lblock) + get_size(rblock);
	create_block(lblock, size);
}

void split_block(WORD* block, WORD lsize) {
	WORD size = get_size(block);

	if(lsize > size || lsize <= 0 || lsize % BLOCK_ALIGN != 0) {  
		printf("ERROR: Invalid value of lsize passed to split_block()\n");
		exit(1);
	}

	create_block(block, lsize);
	create_block(block + lsize, size - lsize);
}




// An implementation of init_heap() using create_block()

void init_heap(struct Heap* H) {
	memset(H->bytes, 0, sizeof(BYTE) * H->size * WORD_SIZE);
	
	WORD* block = (WORD*) H->bytes;
	create_block(block, H->size);  
	H->head_free = block;
	H->next_free = block;
}






// Functions for traversing through a heap block by block

WORD* next_block(WORD* block) {
	return block + get_size(block);
}


WORD* first_block(struct Heap* H) {
	return (WORD*) H->bytes;
}


int at_heap_end(struct Heap *H, WORD* block) {	
	if(block - first_block(H) < H->size) 
		return 0;
	else
		return 1;
}

int at_heap_start(struct Heap* H, WORD* block) {
	if(block == first_block(H))
		return 1;
	else
		return 0;
}



// Resetting the heap to the first free block

void reset_heap(struct Heap* H) {
    WORD* B;
    for(B = first_block(H); !at_heap_end(H, B); B = next_block(B))
        if(is_free(B))
            break;
    
    H->next_free = at_heap_end(H, B) ? NULL : B;
}


// Determining the minimum size of a block, given its payload size

extern int ALLOC_FIXED_SIZE; 
extern int FREE_FIXED_SIZE;


WORD block_size_of_payload(WORD size) {
	size += ALLOC_FIXED_SIZE; //Adding the fixed part
	
	if(size % BLOCK_ALIGN != 0)  //Accounting for alignment
		size += (BLOCK_ALIGN - size % BLOCK_ALIGN);

	return size;
}


WORD min_free_block_size() {
	WORD size = FREE_FIXED_SIZE + 1;

	if(size % BLOCK_ALIGN != 0)
		size += (BLOCK_ALIGN - size % BLOCK_ALIGN);

	return size;
}



// The basic allocator and deallocator used to build the actual alloc() and free()

WORD* alloc_from_block(struct Heap* H, WORD* block, WORD alloc_size);

void free_block(struct Heap *H, WORD* block);





// Abstract functions for traversing through some arbitrary list of blocks (for the policy functions)

WORD* traverse_start(struct Heap* H);

WORD* traverse_next(struct Heap* H, WORD* B);

WORD* traverse_wrap(struct Heap* H, WORD* B);

int traverse_over(struct Heap* H, WORD* B);




// Implementation of various policies for selecting a block for allocation

WORD* first_fit(struct Heap* H, int min_size) {
	for(WORD* B = traverse_start(H); !traverse_over(H, B); B = traverse_next(H, B)) {
		if(!is_free(B))
			continue;

		if(get_size(B) >= min_size)
			return B;
	}

	return NULL;
}


WORD* best_fit(struct Heap* H, int min_size) {
	WORD* best = NULL;

	for(WORD* B = traverse_start(H); !traverse_over(H, B); B = traverse_next(H, B)) {
		if(!is_free(B))
			continue;

		if(get_size(B) >= min_size) {
			if(best == NULL || get_size(B) < get_size(best))
				best = B;
		}
	}

	return best;
}


WORD* next_fit(struct Heap* H, int min_size) {	
	WORD* B = H->next_free;
	if(B == NULL)
		return NULL;

	WORD* start = B;

	int blocks_seen = 0;
	while(! (blocks_seen > 0 && traverse_wrap(H, B) == start) ) {
		if(is_free(B)) {
			if(get_size(B) >= min_size)
				return B;
		}

		B = traverse_wrap(H, B);
		blocks_seen++;
	}

	return NULL;
}




// Functions to handle the relationship between a block and its payload

BYTE* block_to_payload(WORD* block) {
	return (BYTE*) (block + 1);
}

WORD* payload_to_block(BYTE* payload) {
	return (WORD *) payload - 1;
}

WORD payload_size(WORD* block) {
	return get_size(block) - ALLOC_FIXED_SIZE;
}

void clear_payload(WORD* block) {
	int size = get_size(block);
	memset(block_to_payload(block), 0, payload_size(block) * WORD_SIZE);
}




// Utility function to convert bytes to words

WORD bytes_to_words(int bytes) {
	WORD words = bytes / WORD_SIZE;
	if(bytes % WORD_SIZE != 0)
		words++;

	return words;
}




// THe alloc() and free() functions, which are exposed to the user

char* heap_alloc(struct Heap* H, int bytes) {
	WORD size = block_size_of_payload(bytes_to_words(bytes));

	WORD* block = NULL;
	if(H->policy == FIRST_FIT) 
		block = first_fit(H, size);
	else if(H->policy == BEST_FIT)
		block = best_fit(H, size);
	else if(H->policy == NEXT_FIT) 
		block = next_fit(H, size);
	

	if(block == NULL)
		return NULL;

	block = alloc_from_block(H, block, size); 
	clear_payload(block);

	return (char *) block_to_payload(block);	
}

void heap_free(struct Heap* H, char* payload) {
	WORD* block = payload_to_block((BYTE* ) payload);
	free_block(H, block);	
}





// The remaining functions are used for testing purposes only


// Prints the block layout of the heap

void heap_layout(struct Heap* H) {
	printf("Displaying contents of the Heap\n");

	int i = 0;
	for(WORD* B = first_block(H); !at_heap_end(H, B); B = next_block(B)) {
		printf("\tBlock %0*d", 3, i);
		printf(" at address %0*"WFX, WORD_SIZE * 2, (WORD) ( (BYTE* ) B - (BYTE* ) first_block(H)));
		printf(" of size %0*"WFS, 3, get_size(B) * WORD_SIZE);
		printf(" is %s\n", is_free(B) ? "free" : "allocated");

		i++;
	}
}


int resolution(int x, int step) {
	return step * (x / step);
}




// Dumps memory from the heap onto the screen

void heap_dump(struct Heap* H, BYTE* start, int num, int words_per_row) {
	int width = WORD_SIZE * 2;
	int row_size = words_per_row * WORD_SIZE;

	//Printing the header
	printf("%*s ", width + 1, "");
	for(int i = 0; i < words_per_row; i++)
		printf("%-*x ", width, i * WORD_SIZE);
	printf("\n");


	int byte_start = start - H->bytes;
	int byte_end = byte_start + num;

	int row_start = resolution(byte_start, row_size);
	int row_end   = resolution(byte_end - 1, row_size) + row_size;

	
	for(int row = row_start; row < row_end; row += row_size) {
		//Printing the address of the row
		printf("%0*x: ", width, row);

		//Printing the contents of the row
		for(int i = 0; i < words_per_row; i++) {
			int word = row + (i * WORD_SIZE);
				
			if(word/WORD_SIZE < H->size) 
				printf("%0*"WFX" ", width, * (WORD *) (H->bytes + word));
			else {
				for(int i = 0; i < width; i++) 
					putchar('_');
				putchar(' ');
			}
		}
		printf("\n");
	}
}



// An abstract function needed by test_heap() while performing a reset

void split_free_block(struct Heap *H, WORD* B, WORD split_size);




// An REPL testing loop, to facilitate easy testing and viewing

void test_heap(struct Heap* H) {
	char s[50];

	printf("\nRunning testing REPL. Available commands are layout, dump, alloc, free, reset\n");
	while(1) {
		printf("\n>> ");

		scanf("%49s", s);
		
		if(!strcmp(s, "q")) 
			break;

		if(!strcmp(s, "layout")) {
			heap_layout(H);			
			continue;
		}

		if(!strcmp(s, "dump")) {
			int start, end, words_per_row;

			scanf("%s", s);
			if(!strcmp(s, "d"))
				scanf("%d%d%d", &start, &end, &words_per_row);
			else if(!strcmp(s, "x"))
				scanf("%x%x%d", &start, &end, &words_per_row);
			else if(!strcmp(s, "all")) {
				scanf("%d", &words_per_row);

				start = 0;
				end = H->size * WORD_SIZE - 1;
			} 
			
			heap_dump(H, H->bytes + start, end - start,  words_per_row);
			continue;
		}

		if(!strcmp(s, "alloc")) {
			int bytes;
			scanf("%d", &bytes);
			char* p = heap_alloc(H, bytes);

			if(p == NULL) 
				printf("Failed to allocate\n");
			else
				printf("Allocated %d bytes successfully\n", bytes);

			continue;
		}

		if(!strcmp(s, "free")) {
			int num;
			scanf("%d", &num);

			WORD* B = first_block(H);
            int block_num = num;
			while(num-- > 0) {
				if(at_heap_end(H, B)) {
					B = NULL;
					break;
				}

				B = next_block(B);
			}

			if (B == NULL) 
				printf("Invalid block number\n");
			else if (is_free(B)) 
				printf("Block is already free\n");
			else {
				free_block(H, B);
				printf("Freed block no. %d\n", block_num);
			}

			continue;
		}

		if(!strcmp(s, "reset")) {
			scanf("%s", s);

			int unit;
			if(!strcmp(s, "w"))
				unit = WORD_SIZE;
			else
				unit = 1; //byte
				

			init_heap(H);
			WORD* block = first_block(H);

			printf("Enter the block layout. Give 0 as block size when done.\n");
			while(1) {
				int size, a;

				scanf("%d", &size);
				if(size == 0) {	
					printf("Done\n");
					break;
				}

				if(at_heap_end(H, block)) {
					printf("Heap is fully occupied\n");
					break;
				}

				size = (size * unit) / WORD_SIZE;

				scanf("%s", s);
				if(!strcmp(s, "a")) 
					alloc_from_block(H, block, size);
				else
					split_free_block(H, block, size);	
			
				block = next_block(block);
			}

                           //Maybe we need a new NULL value for when no next fit has happened 
                           //(We need to be very careful in the case of explicit free lists)
			reset_heap(H); //This reset function needs some serious reworking, looks hacky. 
			continue;
		}

		printf("?\n");
	}
}



