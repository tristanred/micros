#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "paging.h"
#include "memory_zones.h"

#define MM_HEAP_ALLOC_CANARY_SIZE 5
#define MM_HEAP_ALLOC_CANARY_VALUE "QUACK"

enum mm_alloc_types // TODO DOCUMENT
{
    MEM_UNALLOC,
    MEM_ALLOC,
    MEM_STUB
};

enum mm_alloc_flags
{
    MEM_NOFLAGS = 0,
    MEM_CHECKED = 1, // Alloc has a canary on the last 5 bytes
    MEM_ZEROMEM = 2,
    MEM_PLACEHOLDER = 4
};

enum mm_free_flags
{
    MEM_FREENONE = 0,
    
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

enum heap_flags
{
    FHEAP_NONE = 0
};

struct m_heap
{
    enum heap_flags hflags;
    
    uint32_t startAddress;
    uint32_t endAddress;
    
    size_t allocs_count;
    struct m_allocation* firstAlloc;
    struct m_allocation* lastAlloc;
};

#define HEAP ks_get_current()->task_heap
#define HEAP_LENGTH (ks_get_current()->task_heap->endAddress - ks_get_current()->task_heap->startAddress)

// size_t allocs_count;
// struct m_allocation* firstAlloc;
// struct m_allocation* lastAlloc;

struct memory_manager_module
{
    struct m_heap* kernel_heap;
    
    size_t allocs_count;
    struct m_allocation* allocs;

    struct m_allocation* firstAlloc;
    struct m_allocation* lastAlloc;
};
struct memory_manager_module* mm_module;

// Internal methods

void init_memory_manager(struct kernel_info_block* kinfo);

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

void* kmallocf(uint32_t size, enum mm_alloc_flags f);
void kfreef(void* ptr);

void kmemplace(void* dest, uint32_t offset, const char* data, size_t count);
void kmemget(void* src, char* dest, uint32_t offset, size_t count, size_t* readSize);

// Heap methods
struct m_heap* mm_create_heap(enum heap_flags flags);

// Memory zones
void mm_zone_find_largest(uint32_t* start, uint32_t* length);

// Private Methods

uint32_t mm_get_space(struct m_allocation* first, struct m_allocation* second);

uint32_t mm_data_head(struct m_allocation* target);
uint32_t mm_data_tail(struct m_allocation* target);
uint32_t mm_space_to_end(struct m_allocation* target);

void mm_link_allocs(struct m_allocation* first, struct m_allocation* second);

struct m_allocation* mm_find_free_allocation();
struct m_allocation* mm_find_free_space(size_t bytes);

void  mm_set_alloc_canary(struct m_allocation* alloc);
BOOL mm_verify_alloc_canary(struct m_allocation* alloc);
BOOL mm_verify_all_allocs_canary();

#endif
