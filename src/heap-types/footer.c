#include "../common.h"

int ALLOC_FIXED_SIZE = 2;
int FREE_FIXED_SIZE  = 2;

WORD get_footer(WORD* block) {
	return *(block + get_size(block) - 1);
}




void set_footer(WORD* block, WORD size, int a) {
	*(block + size - 1) = size * WORD_SIZE | a;	
	//*(block + size - 1) = size | a;	
}

void set_a_bit(WORD* block, int a) { 
	set_header(block, get_size(block), a);
	set_footer(block, get_size(block), a);
}





void create_block(WORD* block, WORD size) {
	set_header(block, size, 0);
	set_footer(block, size, 0);
}





WORD* prev_block(WORD* block) {
	WORD* footer = block - 1;
	WORD size = size_from_header(*footer);
	return block - size;
}





WORD* alloc_from_block(struct Heap* H, WORD* block, WORD alloc_size) {
	WORD size = get_size(block);
	WORD min_size = min_free_block_size();

	if(size - alloc_size < min_size) {
		set_a_bit(block, 1);
	} else {
		split_block(block, alloc_size);
		set_a_bit(block, 1);
	}

	H->next_free = traverse_wrap(H, block);

	return block;
}



void free_block(struct Heap *H, WORD* block) {
	set_a_bit(block, 0);
	
	WORD* next = next_block(block);
	if(!at_heap_end(H, next) && is_free(next)) 
		merge_blocks(block, next);

	if(!at_heap_start(H, block)) {
		WORD* prev = prev_block(block);
		if(is_free(prev))
			merge_blocks(prev, block);
	}

}




WORD* traverse_start(struct Heap* H) {
	return first_block(H);
}

WORD* traverse_next(struct Heap* H, WORD* B) {
	return next_block(B);
}

int traverse_over(struct Heap* H, WORD* B) {
	return at_heap_end(H, B);
}

WORD* traverse_wrap(struct Heap* H, WORD* B) {
	B = next_block(B);
	if(at_heap_end(H, B))
		B = first_block(H);

	return B;
}



void split_free_block(struct Heap* H, WORD* B, WORD split_size) {
	split_block(B, split_size);	
}






