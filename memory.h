#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

uint32_t kmNextAvailableMemoryAddress;

/* POOL ALLOCATOR
 *
 *
 */
struct AllocationUnit
{
    uint32_t size;
    void* p;
    BOOL isFree;
};
typedef struct AllocationUnit alloc_unit_t;

struct memstats
{
    size_t small_pool_count;
    size_t small_pool_used;
    size_t small_pool_free;
    
    size_t small_pool_mem_unit;
    size_t small_pool_mem_used;
    size_t small_pool_mem_free;
    
    size_t page_pool_count;
    size_t page_pool_used;
    size_t page_pool_free;
    
    size_t page_pool_mem_unit;
    size_t page_pool_mem_used;
    size_t page_pool_mem_free;
    
    size_t large_pool_count;
    size_t large_pool_used;
    size_t large_pool_free;
    
    size_t large_pool_mem_unit;
    size_t large_pool_mem_used;
    size_t large_pool_mem_free;

    size_t total_alloc_amount;
    size_t total_alloc_used;
    size_t total_alloc_free;

    size_t total_memory_amount;
    size_t total_memory_used;
    size_t total_memory_free;
};

#define small_pool_unit 256
#define page_pool_unit 1024 * 4
#define large_pool_unit 1024 * 32

#define small_pool_size 10 * 1024
#define page_pool_size 610
#define large_pool_size 160

uint32_t basePoolsAddress;

alloc_unit_t smallPool[small_pool_size];
alloc_unit_t pagePool[page_pool_size];
alloc_unit_t largePool[large_pool_size];

// Paging Stuff
#define PAGE_ALL_PRESENT

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t page_tables[1024*1024] __attribute__((aligned(4096)));

void setup_paging();
void switch_phys_address(uint32_t addressFrom, uint32_t addressTo);

void kmInitManager();

void* kmKernelAlloc(size_t size);

void kmKernelFree(void* ptr);

// TODO : Size detection of memory pointed is not yet active, use strcpy if need
// or create a copy method with amount of mem copied.
void kmKernelCopy(void* ptrFrom, void* ptrTo);

void kmKernelZero(void* ptrFrom);


// Pool management method
size_t kmCountFreeSmallPoolUnits();
size_t kmCountFreePagePoolUnits();
size_t kmCountFreeLargePoolUnits();

// Memory statistics methods
struct memstats* kmGetMemoryStats();
char** kmGetMemoryStatsText(int* linesCount);

// User mode memory methods
// Currently define'd while userspace implementation is created
#define malloc kmKernelAlloc
#define free kmKernelFree

#endif
