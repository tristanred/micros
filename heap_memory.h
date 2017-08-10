#ifndef HEAP_MEMORY_H
#define HEAP_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

#define STARTING_ALLOCS_COUNT 100

enum mm_alloc_types
{
    MEM_ALLOC,
    MEM_ALLOCS_BLOCK
};

struct m_allocation
{
    uint32_t size;
    void* p;
    BOOL allocated;
    
    enum mm_alloc_types type;
    uint32_t flags;
    
    struct m_allocation* next;
};

size_t alloc_count;

struct m_allocation* first_alloc;
struct m_allocation* last_alloc;

uint32_t kernel_heap_start;
size_t kernel_heap_size;

// Internal methods

void init_memory_manager();

// Public API

// Standard C methods
void* kmalloc(uint32_t size);
void kfree(void* ptr);
void* krealloc( void *ptr, uint32_t new_size );

// Extended methods
void kmzero(void* ptr);
void* kmemcpy( void *dest, const void *src, uint32_t count );


// Private Methods
uint32_t mm_find_max_memory();

void mm_setup_first_alloc_block();

uint32_t mm_find_free_space(uint32_t sizeNeeded);

uint32_t mm_get_space_between(struct m_allocation* first, struct m_allocation* second);

void* mm_create_alloc(uint32_t startAddress, uint32_t size);

#endif