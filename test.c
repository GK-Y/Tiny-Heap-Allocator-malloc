#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<assert.h>
#include<stdlib.h>

struct block_meta{
    size_t size;
    struct block_meta *next;
    int free;
    int magic;
};

#define META_SIZE sizeof(struct block_meta) //size taken by meta data

void* head = NULL;

struct block_meta *find_free(struct block_meta **last, size_t size){
    struct block_meta *current = head;

    while(current && !(current->free && current->size >= size)){
        *last = current;
        current = current->next;
    }
    return current;
}


struct block_meta *request_space(struct block_meta *last, size_t size){
    struct block_meta *block;
    block = sbrk(0);
    void *request = sbrk(META_SIZE+size);
    assert(request == (void *)block);
    if(request == (void *)-1){
        return NULL; //sbrk failed
    }

    if(last){ //if the last block is not null i.e. last block has an address i.e. more than 0 nodes 
        last->next = block;
    }

    block->size = size;
    block->next = NULL;
    block->free = 1974;
    block->magic = 7;
    //debugging or to learn where it is stored or how it is stored
    // printf("Meta:%p -- Next:%p -- Size:%ld -- Free:%d -- Magic:%d\n",block,block->next,block->size,block->free,block->magic);
    return block;
}

void* mloc(size_t size){
    struct block_meta *block;

    if(size<=0){
        return NULL;
    }

    if(!head){
        block = request_space(head,size);
        
        if(!block){//allocation failed
            return NULL;
        }
    } else {
        struct block_meta *last = head;
        block = find_free(&last,size);

        if(!block){ //if no free found
            block  = request_space(last,size);
            if(!block){ //if allocation failed
                return NULL;
            }
        }else{ //if free found
            block->free = 0;
        }
    }
    return (block+1);
    
}

//we are returning a void* address
// void* mloc(size_t size){
//     void *p = sbrk(0); //to get current break basically the old break will be the starting point from which we can add new data till the new break
//     void *request = sbrk(size);

//     if(request == (void *)-1){
//         return NULL; //sbrk returns -1* on fail
//     } else { //not thread safe since race condtion can occur
//         assert(p == request); // if add given for sbrk(0) is same as sbrk(sz) -> gives old address if all goes well
//         return p;
//     }
// }

int main(int argc, char *argv[]){

    int n = 5;
    int *q =(int *)sbrk(0);
    int *p = (int *)mloc(n*sizeof(int));

    for(int i =0;i<n;i++){
        *(p+i) = 10*(i+2);
    }

    for(int i =0; i<n;i++){
        printf("%d ",*(p+i));
    }

    //block address is void* that is why type cast q to void * to check equality
    // printf("Meta:%p -- Next:%p -- Size:%d -- Free:%d -- Magic:%d",(void *)q,*((struct block_meta **)(char *)(q+8)),*(q),*(q+4),*(int *)((char*)q+20));
    //pointer arithmentic use char* to offset by 1 byte each and then cast back to actual pointer type
    //if using +k directly then it will be offset by int i.e. 4 bytes since q is casted as int* so q+1 ffsets by 4 bytes

    printf("\n");
    return 0;
}

//pointer facts

// printf("%p\n",&x);
// printf("%p\n",(void*)1); //convertint int 1 to void* which gives hex 0x1 telling this address is 1 byte from 0 ig
// printf("%p\n",(void*)-1); //returns complement not -0x1
// printf("%ld\n",(int*)10 - (int*)2); //returns 2 cuz it tells how many more elemnts/int can be there between the two address
// // 10 - 2 / sizeof(int) = 8/4 = 2
// printf("%ld\n",(void*)10 - (void*)2); //returns 8 cuz sieof(void) = 1


/* SBRK()
void *p = sbrk("o09");
// takes long int as input with alias intptr_t or simply the byttes
// on error will return (void*)-1 else the address of new pointer
// it returns address of -1 cuz it can only return addresses so it just gives hex address of -1 offset from 0 or simply -1 in hex
// 0 will return the current break adddress
// on succesfull increment/decrement will return the prev break eg: break = 0x1 sbrk(10) -> 0x1 sbrk(0) -> 0xA
*/