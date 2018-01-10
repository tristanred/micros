#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "paging.h"

// Heap sizes flags
#define KERNEL_HEAP_START (1024 * 1024 * 6)
#define KERNEL_HEAP_LENGTH (25600 * PAGE_SIZE) // 100MB

// Feature flags
#define MM_ZERO_ON_FREE

//#define MM_ENABLE_HEAP_ALLOC_CANARY

#define MM_HEAP_ALLOC_CANARY_SIZE 5
#define MM_HEAP_ALLOC_CANARY_VALUE "QUACK"

enum mm_alloc_types
{
    MEM_UNALLOC,
    MEM_ALLOC,
    MEM_STUB
};

struct m_allocation
{
    uint32_t size;
    void* p;
    BOOL allocated;

    enum mm_alloc_types type;
    uint32_t flags;

    struct m_allocation* previous;
    struct m_allocation* next;
};

size_t allocs_count;
struct m_allocation* allocs;

struct m_allocation* firstAlloc;
struct m_allocation* lastAlloc;

// Internal methods

void init_memory_manager();

// Public API

#define malloc kmalloc
#define free kfree
#define memcpy kmemcpy

// Standard C methods
void* kmalloc(uint32_t size);
void kfree(void* ptr);
void* krealloc( void *ptr, uint32_t new_size );

// Extended methods
void* kmemcpy( void *dest, const void *src, uint32_t count );

void kmemplace(void* dest, uint32_t offset, const char* data, size_t count);
void kmemget(void* src, char* dest, uint32_t offset, size_t count, size_t* readSize);

// Private Methods

uint32_t mm_get_space(struct m_allocation* first, struct m_allocation* second);

uint32_t mm_data_head(struct m_allocation* target);
uint32_t mm_data_tail(struct m_allocation* target);
uint32_t mm_space_to_end(struct m_allocation* target);

void mm_link_allocs(struct m_allocation* first, struct m_allocation* second);

struct m_allocation* mm_find_free_allocation();
struct m_allocation* mm_find_free_space(size_t bytes);

#ifdef MM_ENABLE_HEAP_ALLOC_CANARY
void  mm_set_alloc_canary(struct m_allocation* alloc);

BOOL mm_verify_alloc_canary(struct m_allocation* alloc);

BOOL mm_verify_all_allocs_canary();

#endif

#endif
