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

void kmKernelCopy(void* ptrFrom, void* ptrTo)
{
    char* ptrFromChar = (char*)ptrFrom;
    char* ptrToChar = (char*)ptrTo;
    
    size_t fromSize = sizeof(*ptrFrom);
    
    for(size_t i = 0; i < fromSize; i++)
    {
        ptrToChar[i] = ptrFromChar[i];
    }
}

void kmKernelZero(void* ptrFrom)
{
    char* ptrFromChar = (char*)ptrFrom;
    
    size_t fromSize = sizeof(*ptrFrom);
    
    for(size_t i = 0; i < fromSize; i++)
    {
        ptrFromChar[i] = 0;
    }
}
