#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<assert.h>

// Core Metadata Structure
//implementing a double linked list for efficient and optimal coalescing
//we could do with a single linked list but wont be as quick and can cause issues eg:
// mlc(p q r s) then if we free p then q while freeing we check for coalescing but init p is the only free but after q is freed
//it also thinks its the only free cuz we wont be easily able to check the brevious meta block for coalesincg but would be easy if there were bi directional traversal

typedef struct meta_block{
    size_t size;
    struct meta_block *next;
    struct meta_block *prev;
    int free;
    int debug;
} block_meta_t;

#define META_SIZE sizeof(block_meta_t)
void* head = NULL;

// Request space from the OS
block_meta_t *request_space(block_meta_t* last, int size){
    block_meta_t *block = sbrk(0);
    void *request = sbrk(META_SIZE + size);
    
    assert(block == request);    
    if(request == (void *)-1){
        return NULL;
    }

    if(last){
        last->next = block;
    }

    block->size = size;
    block->next = NULL;
    block->prev = last; //pointing to previous tail block
    block->free = 0;
    block->debug = 999; 

    return block;
}

// Search for a fitting free block
block_meta_t *search_free(block_meta_t **last, int size){
    block_meta_t* current = head;

    while(current && !(current->free && current->size >= size)){
        *last = current;
        current = current -> next;
    }
    return current;
}

// Split a large free block into two
// Split a large free block into two
void split_block(block_meta_t* free_block, int size){
    printf("Splitting block...\n");
    block_meta_t* block;
    
    int remaing_size = free_block->size - size;

    // Calculate address of new metadata block
    block = (block_meta_t*)((char *)free_block + META_SIZE + size); 
    
    // Insertion (Link new block into the list)
    if(free_block->next){
    (free_block->next)->prev = block; //next block's prev points to new block only when free-block->next aint null i.e. not the last block
    }
    block->next = free_block->next; //block's next is free block's next
    block->prev = free_block; //block's prev points to free
    free_block->next = block;
    
    // Set new block's size and status
    block->size = remaing_size - META_SIZE;
    block->free = 1;
    block->debug = 333;

    // Update original block's size
    free_block->size = size;
}


// Custom malloc
void* mlc(int size){
    block_meta_t* block;

    if(size<=0){
        return NULL;
    }

    if(!head){
        block = request_space(NULL, size);
        if(!block) return NULL;
        head = block;
    } else{
        block_meta_t* last = head; 
        block = search_free(&last, size);

        if(!block){
            block = request_space(last, size);
            if(!block) return NULL;
        } else {
            // Check for minimum space for a split
            if(block->size >= size + META_SIZE){ 
                split_block(block, size);
                // After split, 'block' now manages the allocated space
                // The new free block is linked after it.
            }
            // Mark the allocated block as used
            block->free = 0;
            block->debug = 222; 
        }
    }

    return (block+1);
}

// Custom free
void free_mlc(void* block){
    block_meta_t* meta_block;

    // Find metadata via pointer arithmetic
    meta_block = (block_meta_t*)((char *)block - META_SIZE);

    meta_block->free = 1;
    meta_block->debug = 777;

    // TODO: Coalescing logic goes here
}


int main(int argc, char* argv[]) {
    



    return 0;
}