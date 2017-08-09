#ifndef HEAP_MEMORY_H
#define HEAP_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

#define STARTING_ALLOC_COUNT 100

struct m_allocation
{
    uint32_t size;
    void* p;
    BOOL allocated;
};

size_t alloc_count;
size_t alloc_vector_total_size;
struct m_allocation* alloc_vector;

// Internal methods

void init_memory_manager();

// Public API

// Standard C methods
void* kmalloc(size_t size);
void kfree(void* ptr);
void* krealloc( void *ptr, size_t new_size );

// Extended methods
void kmzero(void* ptr);
void* kmemcpy( void *dest, const void *src, size_t count );


// Private Methods
struct m_allocation* mm_find_free_alloc_slot(size_t size);

#endif