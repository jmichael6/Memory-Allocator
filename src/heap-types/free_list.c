#include "../common.h"

int ALLOC_FIXED_SIZE = 2; //What to really do about this ? Probably the best solution
int FREE_FIXED_SIZE = 4;

WORD HEAP_NULL = ~0;


WORD get_footer(WORD* block) {
	return *(block + get_size(block) - 1);
}

WORD get_prev(WORD* block) {
	return *(block + 1);
}

WORD get_next(WORD* block) {
	return *(block + 2);
}





void set_footer(WORD* block, WORD size, int a) {
	*(block + size - 1) = size | a;	
}

void set_a_bit(WORD* block, int a) { 
	set_header(block, get_size(block), a);
	set_footer(block, get_size(block), a);
}

void set_prev(WORD* block, WORD prev) {
	*(block + 1) = prev;
}

void set_next(WORD* block, WORD next) {
	*(block + 2) = next;
}




//Assuming that alignment will be 4 words or equivalent
void create_block(WORD* block, WORD size) {
	set_header(block, size, 0);
	set_footer(block, size, 0);
	set_prev(block, HEAP_NULL);
	set_next(block, HEAP_NULL);
}




WORD* prev_block(WORD* block) {
	WORD* footer = block - 1;
	WORD size = size_from_header(*footer);
	return block - size;
}






WORD* block_pointer(struct Heap* H, WORD addr) {  
	if(addr == HEAP_NULL)
		return NULL;
	else
		return (WORD* ) (H->bytes + addr);
}


WORD block_address(struct Heap* H, WORD* B) {
	if(B == NULL)
		return HEAP_NULL;
	else 
		return (BYTE* ) B - H->bytes;
}


void link_blocks(struct Heap *H, WORD* A, WORD* B) {  
	if(A != NULL) 
		set_next(A, block_address(H, B));

	if(B != NULL) 
		set_prev(B, block_address(H, A));
}






WORD* traverse_start(struct Heap* H) {
	return H->head_free;
}

WORD* traverse_next(struct Heap* H, WORD* B) {
	return block_pointer(H, get_next(B));
}

WORD* traverse_wrap(struct Heap* H, WORD* B) {
	if(B == NULL)
		return NULL;

	WORD* next = block_pointer(H, get_next(B));
	if(next == NULL)
		return H->head_free;
	else
		return next;
}

int traverse_over(struct Heap* H, WORD* B) {
	return B == NULL;
}




void reduce_block(struct Heap* H, WORD* B, int alloc_size) {
	WORD prev = get_prev(B);
	WORD next = get_next(B);

	if(alloc_size == get_size(B)) {
		link_blocks(H, block_pointer(H, prev), block_pointer(H, next));

		if(B == H->head_free)
			H->head_free = block_pointer(H, get_next(B));

	} else if (alloc_size < get_size(B)) {
		split_block(B, alloc_size);

		link_blocks(H, block_pointer(H, prev), B + alloc_size);
		link_blocks(H, B + alloc_size, block_pointer(H, next));

		if(B == H->head_free)							  
			H->head_free = B + alloc_size;
	}
}



WORD* alloc_from_block(struct Heap* H, WORD* block, WORD alloc_size) {
	WORD size = get_size(block);
	WORD min_size = min_free_block_size();

	if(size - alloc_size < min_size) {
		reduce_block(H, block, size);
		H->next_free = traverse_wrap(H, block);  //We traverse and wrap
	} else {
		reduce_block(H, block, alloc_size);
		H->next_free = block + alloc_size;
	}
	
	set_a_bit(block, 1);                  

	return block;
}


void extend_block(struct Heap *H, WORD* A, WORD* B, WORD* which_one) { //which one is being extended
	WORD prev, next;

	if(which_one == A) {
		prev = get_prev(A);
		next = get_next(A);
	} else if (which_one == B) {
		prev = get_prev(B);
		next = get_next(B);
	}

	merge_blocks(A, B);

	link_blocks(H, block_pointer(H, prev), A);
	link_blocks(H, A, block_pointer(H, next));

	if(which_one == B && B == H->head_free)
		H->head_free = A;
			
}



void free_block(struct Heap *H, WORD* block) {
	set_a_bit(block, 0);
	
	WORD* next = next_block(block);
	if(!at_heap_end(H, next) && is_free(next)) {
		extend_block(H, block, next, next);

	} else if(!at_heap_start(H, block)) {
		WORD* prev = prev_block(block);
		if(is_free(prev)) 
			extend_block(H, prev, block, prev);
	} else {
		if(H->head_free == NULL)  
			H->head_free = H->next_free = block;
		else { // We are pushing the new block onto the head. If we want, we can do something different
			link_blocks(H, block, H->head_free);
			H->head_free = block;
		}
	}

}




void split_free_block(struct Heap *H, WORD* B, WORD split_size) {
	if(split_size == get_size(B))
		return;

	WORD prev = get_prev(B);
	WORD next = get_next(B);

	split_block(B, split_size);

	link_blocks(H, block_pointer(H, prev), B);
	link_blocks(H, B, B + split_size);
	link_blocks(H, B + split_size, block_pointer(H, next));
}



