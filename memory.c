#include "memory.h"

void kmInitManager()
{
    // Start the allocations at the 45MB mark
    kmNextAvailableMemoryAddress = 45 * (1024 * 1024);
}

void* kmKernelAlloc(size_t size)
{
    kmNextAvailableMemoryAddress += size;
    return (void*)kmNextAvailableMemoryAddress;
}

void kmKernelFree(void* ptr)
{
    ptr = ptr;
}
