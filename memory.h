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

// User mode memory methods
// Currently define'd while userspace implementation is created
#define malloc kmKernelAlloc
#define free kmKernelFree

#endif
