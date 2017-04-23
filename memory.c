#include "memory.h"

#include "kernel_log.h"
#include "common.h"

void kmInitManager()
{
    basePoolsAddress = 1024 * 1024 * 5; // 1MB
    
    uint32_t smallPoolStartAddress = basePoolsAddress;
    uint32_t pagePoolStartAddress = smallPoolStartAddress + small_pool_size * small_pool_unit;
    uint32_t largePoolStartAddress = pagePoolStartAddress + page_pool_size + page_pool_unit;
    
    for(int i = 0; i < small_pool_size; i++)
    {
        smallPool[i].size = 256;
        smallPool[i].isFree = TRUE;
        smallPool[i].p = (void*)(basePoolsAddress + 256 * i);
    }
    
    for(int i = 0; i < page_pool_size; i++)
    {
        pagePool[i].size = 4096;
        pagePool[i].isFree = TRUE;
        pagePool[i].p = (void*)(pagePoolStartAddress + 4096*i);
    }
    
    for(int i = 0; i > large_pool_size; i++)
    {
        largePool[i].size = 32 * 1024;
        largePool[i].isFree = TRUE;
        largePool[i].p = (void*)(largePoolStartAddress + 32 * 1024 * i);
    }
    
    // Start the allocations at the 45MB mark
    kmNextAvailableMemoryAddress = 45 * (1024 * 1024);
}

void* kmKernelAlloc(size_t size)
{
    // kmNextAvailableMemoryAddress += size;
    // return (void*)kmNextAvailableMemoryAddress;
    //
    if(size <= small_pool_unit)
    {
        for(int i = 0; i < small_pool_size; i++)
        {
            if(smallPool[i].isFree)
            {
                smallPool[i].isFree = FALSE;
                
                return smallPool[i].p;
            }
        }
        
        kWriteLog("Small pool is full !");
    }
    else if(size <= page_pool_unit)
    {
        for(int i = 0; i < page_pool_size; i++)
        {
            if(pagePool[i].isFree)
            {
                pagePool[i].isFree = FALSE;
                
                return pagePool[i].p;
            }
        }
        
        kWriteLog("Page pool is full !");
    }
    else if(size <= large_pool_unit)
    {
        for(int i = 0; i < large_pool_size; i++)
        {
            if(largePool[i].isFree)
            {
                largePool[i].isFree = FALSE;
                
                return largePool[i].p;
            }
        }
        
        kWriteLog("Large pool is full !");
    }
    else
    {
        // Allocation is too big, not supported for now.
        kWriteLog("Trying to allocate too much.");
        
        Debugger();
    }
    
    
    return (void*)0;
    // kmNextAvailableMemoryAddress += size;
    // return (void*)kmNextAvailableMemoryAddress;
}

void kmKernelFree(void* ptr)
{
    // TODO : For now, no other way than checking each allocation unit
    
    for(int i = 0; i < large_pool_size; i++)
    {
        if(largePool[i].p == ptr)
        {
            largePool[i].isFree = TRUE;
            
            return;
            // Maybe zero out the memory or something.
        }
    }
    
    for(int i = 0; i < page_pool_size; i++)
    {
        if(pagePool[i].p == ptr)
        {
            pagePool[i].isFree = TRUE;
            
            return;
        }
    }

    for(int i = 0; i < small_pool_size; i++)
    {
        if(smallPool[i].p == ptr)
        {
            smallPool[i].isFree = TRUE;
            
            return;
        }
    }
    
    kWriteLog_format1d("Could not find allocation for %d", (uint32_t)ptr);
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


size_t kmCountFreeSmallPoolUnits()
{
    size_t total = 0;
    
    for(int i = 0; i < small_pool_size; i++)
    {
        if(smallPool[i].isFree)
        {
            total++;
        }
    }
    
    return total;
}

size_t kmCountFreePagePoolUnits()
{
    size_t total = 0;
    
    for(int i = 0; i < page_pool_size; i++)
    {
        if(pagePool[i].isFree)
        {
            total++;
        }
    }
    
    return total;
}

size_t kmCountFreeLargePoolUnits()
{
    size_t total = 0;
    
    for(int i = 0; i < large_pool_size; i++)
    {
        if(largePool[i].isFree)
        {
            total++;
        }
    }
    
    return total;
}
