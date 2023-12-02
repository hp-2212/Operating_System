#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

#define PAGE_SIZE 4096
#define MAX_REQUESTS 100

long long totalAllocatedBlockSize = 0 ;
int totalAllocatedBlockCount = 0 ;

typedef struct {
    int byteLength ;
    struct Node* nextNode ;
} Node ;

Node* freeListHead = NULL ;

void debug_heap() {
    fprintf(stderr, "Heap Information:\n");
    fprintf(stderr, "-----------------\n");
    fprintf(stderr, "Total heap size: %lld bytes\n", totalAllocatedBlockSize);
    fprintf(stderr, "Number of allocated blocks: %d\n", totalAllocatedBlockCount);
    fprintf(stderr, "List of available Free blocks:\n");

    // Printing free List
    Node* temp = freeListHead ;
    printf("After Init ... Printing Free List \n") ;
    while ( temp != NULL ) {
        printf("Address0 =  %p\n", temp) ;
        printf("Size = %d\n", temp->byteLength) ;
        temp = temp -> nextNode ;
    }
}


void init() {
    size_t size = 256 * PAGE_SIZE;
    freeListHead = (Node*) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    freeListHead->byteLength = size - sizeof(Node) ;
    freeListHead->nextNode = NULL ;

    if (freeListHead == MAP_FAILED) {
        perror("mmap");
        return ;
    }

    // Printing free List
    // Node* temp = freeListHead ;
    // printf("After Init ... Printing Free List \n") ;
    // while ( temp != NULL ) {
    //     printf("Address0 =  %p\n", temp) ;
    //     printf("Size = %d\n", temp->byteLength) ;
    //     printf("Size of Node = %ld\n", sizeof(Node)) ;
    //     temp = temp -> nextNode ;
    // }
}

void coalesce() {
    Node* temp = freeListHead ;
    while(temp->nextNode!=NULL){
        Node* next = temp->nextNode;
        if( (char*)temp + sizeof(Node) + temp->byteLength == (char*)next ) {
            temp->byteLength += next->byteLength + sizeof(Node);
            temp->nextNode = next->nextNode;
        }
        else temp = next;
    }

    // printf("After Coalesce ... Printing Free List \n\n") ;
    // temp = freeListHead;
    // while ( temp != NULL ) {
    //     printf("Address0 =  %p\n", temp) ;
    //     printf("Size = %d\n\n", temp->byteLength) ;
    //     temp = temp -> nextNode;
    // }
}

Node* split(Node* bestFit, Node* bestFitPrev, size_t size_required){
    if ( bestFit->byteLength - size_required <= sizeof(Node) ) {
        if ( bestFitPrev ) bestFitPrev->nextNode = bestFit->nextNode ;
        else freeListHead = bestFit->nextNode ;

        totalAllocatedBlockSize += bestFit->byteLength ;
        totalAllocatedBlockCount++ ;

        return bestFit ;
    }

    Node* spiltNode = (Node*)( (char*) bestFit + sizeof(Node) + size_required ) ;
    spiltNode->byteLength = bestFit->byteLength - sizeof(Node) - size_required;
    spiltNode->nextNode = bestFit->nextNode;
    
    bestFit->byteLength = size_required;
    bestFit->nextNode = NULL;

    if ( bestFitPrev != NULL ) bestFitPrev->nextNode = spiltNode ;
    else freeListHead = spiltNode ;

    totalAllocatedBlockSize += bestFit->byteLength ;
    totalAllocatedBlockCount++ ;

    // Printing free List
    // Node* temp = freeListHead ;
    // printf("After Split ... Printing Free List \n") ;
    // while ( temp != NULL ) {
    //     printf("Address2 =  %p\n", temp) ;
    //     printf("Size = %d\n", temp->byteLength) ;
    //     printf("Size of Node = %ld\n", sizeof(Node)) ;
    //     temp = temp -> nextNode ;
    // }

    return bestFit ;
}

void* my_malloc(size_t size) {
    
    if ( freeListHead == NULL ) init() ;

    // Node* temp = freeListHead ;
    // printf("In malloc call ... Printing Free List \n") ;
    // while ( temp != NULL ) {
    //     printf("Address1 =  %p\n", temp) ;
    //     printf("Size = %d\n", temp->byteLength) ;
    //     temp = temp -> nextNode ;
    // }

    Node* currNode = freeListHead ;
    Node* prevNode = NULL ;

    Node* bestFit = NULL ;
    Node* bestFitPrev = NULL ;

    while ( currNode != NULL  ) {
        if ( currNode->byteLength >= size ) {
            if ( bestFit == NULL ) {
                bestFit = currNode ;
                bestFitPrev = prevNode ;
            }
            else {
                if ( bestFit->byteLength <= currNode->byteLength ) ;
                else {
                    bestFit = currNode ;
                    bestFitPrev = prevNode ;
                }
            }
        }
        prevNode = currNode ;
        currNode = currNode->nextNode ;
    }

    if ( bestFit != NULL ) return (char*) split( bestFit, bestFitPrev, size) + sizeof(Node) ;
    else {
        size_t newAllocationSize = 256 * PAGE_SIZE;
        currNode = (Node*) mmap(NULL, newAllocationSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        currNode->byteLength = newAllocationSize - sizeof(Node) ;
        currNode->nextNode = NULL ;
        prevNode->nextNode = currNode ;

        return (char*) split( currNode, prevNode, size) + sizeof(Node) ;
    }
}


void* my_calloc(size_t nelem, size_t size) {
    if ( nelem == 0 || size == 0 ) return NULL ;
    size_t callSize = nelem*size ;
    void* callPtr =  my_malloc(callSize) ;

    if ( callPtr != NULL ) memset( callPtr, 0, callSize) ;
    if ( callSize / nelem != size ) return NULL ;
    return callPtr ;
}

void my_free(void* ptr) {
    if (ptr == NULL) return ;

    Node* newNode = (Node*)(ptr - sizeof(Node)) ;

    totalAllocatedBlockSize -= newNode->byteLength ;
    totalAllocatedBlockCount-- ;

    if ( newNode < freeListHead ) { 
        newNode->nextNode = freeListHead ;
        freeListHead = newNode ;
    }
    else {
        Node* temp = freeListHead ;
        while ( temp->nextNode != NULL && temp->nextNode < newNode ) 
            temp = temp->nextNode ;

        newNode->nextNode = temp->nextNode ;
        temp->nextNode = newNode ;
    }

    coalesce() ;

    // printf("New Node Position = %p\n", newNode) ;
    // printf("Size of freed space = %d\n", newNode->byteLength) ;

    // Printing free List
    // Node* temp = freeListHead ;
    // printf("After Freeing ... Printing Free List \n") ;
    // while ( temp != NULL ) {
    //     printf("Address3 =  %p \n", temp) ;
    //     printf("Size = %d \n", temp->byteLength) ;
    //     temp = temp -> nextNode ;
    // }
}



