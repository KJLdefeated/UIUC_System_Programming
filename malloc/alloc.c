/**
 * malloc
 * CS 341 - Fall 2024
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

struct metadata {
    size_t size;
    struct metadata *next;
    struct metadata *prev;
    int free;
};

static struct metadata *head;
static struct metadata *tail;

static size_t total_memory_use = 0;

#define INITIAL_HEAP_SIZE (256LL * 1024 * 1024)
#define REQUEST_HEAP_SIZE (1LL * 1024 * 1024)
#define MAX_HEAP_SIZE (2500ULL * 1024 * 1024)
#define COALESCE_THRESHOLD 10
#define align_to 8

// static int free_count = 0;

void init_head() {
    head = sbrk(0);
    if (sbrk(INITIAL_HEAP_SIZE) == (void*)-1) {
        perror("sbrk");
        exit(1);
    }
    total_memory_use += INITIAL_HEAP_SIZE;
    head->size = INITIAL_HEAP_SIZE - sizeof(struct metadata);
    head->next = NULL;
    head->prev = NULL;
    head->free = 1;
    tail = head;
}

struct metadata* best_fit(struct metadata *cur, size_t size){;
    cur = head;
    struct metadata *best = NULL;
    size_t min_size = MAX_HEAP_SIZE;
    while (cur != NULL){
        if (cur->free && cur->size >= size && cur->size < min_size){
            best = cur;
            min_size = cur->size;
        }
        cur = cur->next;
    }
    return best;
}

struct metadata* first_fit(struct metadata *cur, size_t size){
    cur = head;
    while (cur != NULL){
        if (cur->free && cur->size >= size){
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

struct metadata* find_last(struct metadata *cur){
    while (cur->next != NULL){
        cur = cur->next;
    }
    return cur;
}

void coalesce(){
    struct metadata *cur = head;
    while (cur != NULL){
        if (cur->free && cur->next && cur->next->free){
            cur->size += cur->next->size + sizeof(struct metadata);
            cur->next = cur->next->next;
            if (cur->next){
                cur->next->prev = cur;
            }
            if (cur->next == NULL){
                tail = cur;
            }
        }
        cur = cur->next;
    }
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    // implement malloc!
    struct metadata *cur=NULL;
    if (head == NULL){
        init_head();
        if (head == NULL){
            return NULL;
        }
    }
    
    size = (size + (align_to - 1) & ~ (align_to - 1));

    // Find best fit
    struct metadata *best;
    
    if (tail->free && tail->size >= size) {
        best = tail;
    }
    else {
        //best = best_fit(cur, size);
        best = first_fit(cur, size);
    }

    if (best){
        if(best->size == size){
            best->free = 0;
            return (void*)(best + 1);
        }
        if (best->size >= size + sizeof(struct metadata) + align_to){
            struct metadata *new = (struct metadata*)((void*)(best + 1) + size);
            new->size = best->size - size - sizeof(struct metadata);
            new->next = best->next;
            new->prev = best;
            new->free = 1;
            best->next = new;
            if (best == tail){
                tail = new;
            }
        }
        best->size = size;
        best->free = 0;
        return (void*)(best + 1);
    }
    else{
        cur = sbrk(0);
        if (sbrk(size + sizeof(struct metadata)) == (void*)-1){
            return NULL;
        }
        cur->size = size;
        cur->next = NULL;
        cur->prev = tail;
        cur->free = 0;
        tail->next = cur;
        tail = cur;
        return (void*)(cur + 1);
    }
    return NULL;
}

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
    size_t total_size;
    void *ptr;
    // Check for multiplication overflow
    if (num!=0 && size > SIZE_MAX / num){
        return NULL;
    }
    total_size = num * size;
    ptr = malloc(total_size);
    if (ptr){
        memset(ptr, 0, total_size);
    }
    return ptr;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    // implement free!
    struct metadata *cur, *prev, *next;
    if (ptr == NULL){
        return;
    }
    cur = (struct metadata*)ptr - 1;
    cur->free = 1;
    prev = cur->prev;
    next = cur->next;
    if (prev && prev->free){
        prev->size += cur->size + sizeof(struct metadata);
        prev->next = next;
        if (next){
            next->prev = prev;
        }
        if (cur == tail){
            tail = prev;
        }
        cur = prev;
    }
    if (next && next->free){
        cur->size += next->size + sizeof(struct metadata);
        cur->next = next->next;
        if (next->next){
            next->next->prev = cur;
        }
        if (next == tail){
            tail = cur;
        }
    }
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
    if (ptr == NULL){
        return malloc(size);
    }

    if (size == 0){
        free(ptr);
        return NULL;
    }

    struct metadata *cur;
    cur = (struct metadata*)ptr - 1;
    if (cur->size >= size){
        return ptr;
    }

    void *new_ptr = malloc(size);
    if (new_ptr){
        memcpy(new_ptr, ptr, cur->size);
        free(ptr);
        return new_ptr;
    }
    return NULL;
}