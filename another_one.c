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
    int size;
    struct meta_block *next;
    struct meta_block *prev;
    int free;
    int debug;
} block_meta_t;

#define META_SIZE sizeof(block_meta_t)
block_meta_t* head = NULL;

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

    //coalescing
    //we will merge the adjacent free of current free block into one big block
    
    //we need to do proper null check cuz free can be head or tail so the next or prev may not exsist
    //or the next next can also not exsist
    block_meta_t* prev_meta = meta_block->prev;
    block_meta_t* next_meta = meta_block->next;
    
    //if prev is free then allow overwrtiting in current new free i.e. remove current meta from meta linked list
    //we already have the value of next meta so we can safely remove it
    //also we shd check if next meta exsits and prev meta exsists
    if(prev_meta){
        //do not check both nulll and free else null_> free will give segfault
        if(prev_meta->free){ 
            //if next meta exsists 
            if(next_meta){
                if(next_meta->free){

                    //first increase size of prev by current free and next free
                    prev_meta->size += 2*META_SIZE + meta_block->size + next_meta->size;

                    //we check if next exsists for next_meta block
                    if(next_meta->next){
                        prev_meta->next = next_meta->next;
                        (next_meta->next)->prev = prev_meta;
                        }else{
                            prev_meta->next = NULL;
                        }
                
                    return; //triple merge finished
                    }

                }

            //if only current and prev meta free and exsists
            prev_meta->size += META_SIZE + meta_block->size;
            prev_meta->next = next_meta;
            if(next_meta) next_meta->prev = prev_meta; //if next meta exsists but isnt free we also check its exsistence
            return;
        }
    }


    //if only current and next meta free then remove next meta i.e. expand current meta
    //incase next meta is null else we get a seg fault
    if(next_meta){
        if(next_meta->free){
            meta_block->size += META_SIZE + next_meta->size;

            meta_block->next = next_meta->next;
            if((next_meta->next)) //if next_meta's next exsists
            (next_meta->next)->prev = meta_block;
        }
    }

}

void print_meta(){
    block_meta_t* current = head;
    while(current){
        printf("(size:%d free:%d) -> ",current->size,current->free);
        current = current->next; 
    }
    printf("END\n");
}

int main(int argc, char* argv[]) {
    
    printf("META_SIZE:%ld\n",META_SIZE);
    int n = 2;
    int* p = (int*)mlc(n*4);
    
    n = 20;
    int* q = (int*)mlc(n*4);
    print_meta();


    n = 1;
    int* r = (int*)mlc(n*4);
    print_meta();
    free_mlc(r);
    print_meta();
    n = 5;
    int* s = (int*)mlc(n*4);
    print_meta();
    free_mlc(s);
    print_meta();

    printf("\n");

    return 0;
}