#include "memory.h"

#include "kernel.h"
#include "kernel_log.h"
#include "common.h"
#include "string.h"

extern void set_paging();
extern void enablePaging();
// extern void invalidateEntry(uint32_t address);

void setup_paging()
{
    int a = 0;
    int addr = 0;
    for(int k = 0; k < 1024; k++)
    {
        for(int i = 0; i < 1024; i++)
        {
            // As the address is page aligned, it will always leave 12 bits zeroed.
            // Those bits are used by the attributes ;)
            defaultPageTable.page_tables[a++] = addr | 3; // attributes: supervisor level, read/write, present.
            
            addr += 0x1000; // Target the next 4KB page.
        }
        
        #ifdef PAGE_ALL_PRESENT
        // Currently for debugging, we'll identity-map all the pages to the 
        // physical address.
        
        // attributes: supervisor level, read/write, present
        defaultPageTable.page_directory[k] = ((uint32_t)&defaultPageTable.page_tables[k * 1024]) | 3;
        
        #else
        
        // If not, we must map the first 8MB (first 2 page directories) to
        // be present because most of the OS currently lives under the 8 first
        // MB's. Rest of the pages are marked not-present to allow testing
        // page faults.
        
        if(k <= 1)
        {
            page_directory[k] = ((uint32_t)&defaultPageTable.page_tables[k * 1024]) | 3;
        }
        else
        {
            page_directory[k] = ((uint32_t)&defaultPageTable.page_tables[k * 1024]) | 2;
        }

        #endif
        
    }
    
    set_paging(defaultPageTable.page_directory);
    enablePaging();
}

void test_paging()
{
    char* far_address = (char*)0x3C00000; // 60 MB
    char* close_address = (char*)0xF00000; // 15 MB
    
    strcpy(far_address, "far_address\0");
    strcpy(close_address, "close_address\0");
    
    map_phys_address(0x3C00000, 0xF00000); // Map 60 MB mark to 15 MB mark.
    
    // Write the string to the address 0x3C00000, which goes over to 0xF00000
    // So far_address still have the old 'far_address' string.
    strcpy(far_address, "xx_far_address_after_mapping\0");
    
    // Both addresses should have the same content since they are mapped to the 
    // same page.
    int res = strcmp(close_address, far_address) == 0;
    ASSERT(res == TRUE, "PAGING IS FUCKED UP");

}

void map_phys_address(uint32_t addressFrom, uint32_t addressTo)
{
    // Take top 10 bits to identify the page directory
    uint32_t upper10 = addressFrom & 0xFFC00000;
    uint32_t pdeIndex = upper10 >> 22;
    
    // Take the middle 10 bits to identify the page table (of the directory above)
    uint32_t lower10 = addressFrom & 0x3FF000;
    uint32_t pte = (lower10 >> 12) + (pdeIndex * 1024);
    
    // Assign the 12 low bits from the target with the flags Present and R/W.
    defaultPageTable.page_tables[pte] = (addressTo & 0xFFFFF000) | 3;
    
    // I'm invalidating both addresses just in case, will test for validity.
    asm volatile("invlpg (%0)" ::"r" (addressFrom) : "memory");
    asm volatile("invlpg (%0)" ::"r" (addressTo) : "memory");
}

int count_pages(enum page_frame_flags findFlags)
{
    int totalCount = 0;
    for(int i = 0; i < 1024*1024; i++)
    {
        if(kMemoryManager->currentPageTable->page_tables[i] & findFlags)
            totalCount++;
    }
    
    return totalCount;
}

uint32_t* find_pages(enum page_frame_flags findFlags, int* count)
{
    (void)findFlags;
    (void)count;
    
    return NULL;
}

void init_module_memory_manager(struct kernel_info_block* kinfo)
{
    kMemoryManager = alloc_kernel_module(sizeof(struct memory_manager));
    kinfo->m_memory_manager = kMemoryManager;
    
    
}

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
    
    for(int i = 0; i < large_pool_size; i++)
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
                smallPool[i].size = size;
                
                return smallPool[i].p;
            }
        }
        
        Debugger();
        kWriteLog("Small pool is full !");
    }
    else if(size <= page_pool_unit)
    {
        for(int i = 0; i < page_pool_size; i++)
        {
            if(pagePool[i].isFree)
            {
                pagePool[i].isFree = FALSE;
                pagePool[i].size = size;
                
                return pagePool[i].p;
            }
        }
        
        Debugger();        
        kWriteLog("Page pool is full !");
    }
    else if(size <= large_pool_unit)
    {
        for(int i = 0; i < large_pool_size; i++)
        {
            if(largePool[i].isFree)
            {
                largePool[i].isFree = FALSE;
                largePool[i].size = size;
                
                return largePool[i].p;
            }
        }
        
        Debugger();
        kWriteLog("Large pool is full !");
    }
    else
    {
        // Allocation is too big, not supported for now.
        kWriteLog("Trying to allocate too much.");
        
        Debugger();
    }
    
    return (void*)0;
}

void kmKernelFree(void* ptr)
{
    // TODO : For now, no other way than checking each allocation unit
    
    for(int i = 0; i < large_pool_size; i++)
    {
        if(largePool[i].p == ptr)
        {
            largePool[i].isFree = TRUE;
            largePool[i].size = 0;
            
            return;
            // Maybe zero out the memory or something.
        }
    }
    
    for(int i = 0; i < page_pool_size; i++)
    {
        if(pagePool[i].p == ptr)
        {
            pagePool[i].isFree = TRUE;
            pagePool[i].size = 0;
            
            return;
        }
    }

    for(int i = 0; i < small_pool_size; i++)
    {
        if(smallPool[i].p == ptr)
        {
            smallPool[i].isFree = TRUE;
            smallPool[i].size = 0;
            
            return;
        }
    }
    
    kWriteLog_format1d("Could not find allocation for %d", (uint32_t)ptr);
}

void kmKernelCopy(void* ptrFrom, void* ptrTo)
{
    uint8_t* ptrFromChar = (uint8_t*)ptrFrom;
    uint8_t* ptrToChar = (uint8_t*)ptrTo;
    
    alloc_unit_t alloc;
    BOOL ptrFound = kmFindInPools(ptrFrom, &alloc);
    
    if(ptrFound)
    {
        for(size_t i = 0; i < alloc.size; i++)
        {
            ptrToChar[i] = ptrFromChar[i];
        }
    }
    else
    {
        ASSERT(FALSE, "kmKernelCopy ptrFrom not found in the pools.");
    }
}

void* memcpy( void *dest, const void *src, size_t count )
{
    uint8_t* ptrSrc = (uint8_t*)src;
    uint8_t* ptrDest = (uint8_t*)dest;
    
    for(size_t i = 0; i < count; i++)
    {
        ptrDest[i] = ptrSrc[i];
    }
    
    return dest;
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

BOOL kmFindInPools(void* ptr, alloc_unit_t* alloc)
{
    for(int i = 0; i < small_pool_size; i++)
    {
        if(smallPool[i].p == ptr)
        {
            *alloc = smallPool[i];
            
            return TRUE;
        }
    }
    
    for(int i = 0; i < page_pool_size; i++)
    {
        if(pagePool[i].p == ptr)
        {
            *alloc = pagePool[i];
            
            return TRUE;
        }
    }
    
    for(int i = 0; i < large_pool_size; i++)
    {
        if(largePool[i].p == ptr)
        {
            *alloc = largePool[i];
            
            return TRUE;
        }
    }
    
    alloc = NULL;
    
    return FALSE;
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

struct memstats* kmGetMemoryStats()
{
    struct memstats* stats = kmKernelAlloc(sizeof(struct memstats));
    
    // SMALL POOL
    stats->small_pool_count = small_pool_size;
    stats->small_pool_used = 0;
    stats->small_pool_free = 0;
    stats->small_pool_mem_unit = small_pool_unit;
    stats->small_pool_mem_used = 0;
    stats->small_pool_mem_free = 0;
    for(int i = 0; i < small_pool_size; i++)
    {
        if(smallPool[i].isFree)
        {
            stats->small_pool_free++;
        }
        else
        {
            stats->small_pool_used++;
            stats->small_pool_mem_used += small_pool_unit;
        }
    }
    stats->small_pool_mem_free = (small_pool_size * small_pool_unit) - stats->small_pool_mem_used;
    
    // PAGE POOL
    stats->page_pool_count = page_pool_size;
    stats->page_pool_used = 0;
    stats->page_pool_free = 0;
    stats->page_pool_mem_unit = page_pool_unit;
    stats->page_pool_mem_used = 0;
    stats->page_pool_mem_free = 0;
    for(int i = 0; i < page_pool_size; i++)
    {
        if(pagePool[i].isFree)
        {
            stats->page_pool_free++;
        }
        else
        {
            stats->page_pool_used++;
            stats->page_pool_mem_used += page_pool_unit;
        }
    }
    stats->page_pool_mem_free = (page_pool_size * page_pool_unit) - stats->page_pool_mem_used;

    // LARGE POOL
    stats->large_pool_count = large_pool_size;
    stats->large_pool_used = 0;
    stats->large_pool_free = 0;
    stats->large_pool_mem_unit = large_pool_unit;
    stats->large_pool_mem_used = 0;
    stats->large_pool_mem_free = 0;
    for(int i = 0; i < large_pool_size; i++)
    {
        if(largePool[i].isFree)
        {
            stats->large_pool_free++;
        }
        else
        {
            stats->large_pool_used++;
            stats->large_pool_mem_used += large_pool_unit;
        }
    }
    stats->large_pool_mem_free = (large_pool_size * large_pool_unit) - stats->large_pool_mem_used;
    
    stats->total_alloc_amount = small_pool_size + page_pool_size + large_pool_size;
    stats->total_alloc_used = 0;
    stats->total_alloc_used += stats->small_pool_used;
    stats->total_alloc_used += stats->page_pool_used;
    stats->total_alloc_used += stats->large_pool_used;
    stats->total_alloc_free = stats->total_alloc_amount - stats->total_alloc_used;

    stats->total_memory_amount = (small_pool_unit * small_pool_size) + (page_pool_unit * page_pool_size) + (large_pool_unit * large_pool_size);
    stats->total_memory_used = 0;
    stats->total_memory_used += stats->small_pool_mem_used;
    stats->total_memory_used += stats->page_pool_mem_used;
    stats->total_memory_used += stats->large_pool_mem_used;
    stats->total_memory_free = stats->total_memory_amount - stats->total_memory_used;
    
    return stats;
}

char** kmGetMemoryStatsText(int* linesCount)
{
    int currentLine = 0;
    int nbLinesTotal = 24;
    char** statsLines = kmKernelAlloc(sizeof(char*) * nbLinesTotal); // 24 fields in memstats
    struct memstats* stats = kmGetMemoryStats();
    
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "SMALL POOL COUNT = %d", stats->small_pool_count, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "SMALL POOL USED = %d", stats->small_pool_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "SMALL POOL FREE = %d", stats->small_pool_free, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "SMALL POOL MEM UNIT = %d", stats->small_pool_mem_unit, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "SMALL POOL MEM USED = %d", stats->small_pool_mem_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "SMALL POOL MEM FREE = %d", stats->small_pool_mem_free, NULL);currentLine++;
    
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "PAGE POOL COUNT = %d", stats->page_pool_count, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "PAGE POOL USED = %d", stats->page_pool_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "PAGE POOL FREE = %d", stats->page_pool_free, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "PAGE POOL MEM UNIT = %d", stats->page_pool_mem_unit, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "PAGE POOL MEM USED = %d", stats->page_pool_mem_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "PAGE POOL MEM FREE = %d", stats->page_pool_mem_free, NULL);currentLine++;
    
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "LARGE POOL COUNT = %d", stats->large_pool_count, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "LARGE POOL USED = %d", stats->large_pool_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "LARGE POOL FREE = %d", stats->large_pool_free, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "LARGE POOL MEM UNIT = %d", stats->large_pool_mem_unit, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "LARGE POOL MEM USED = %d", stats->large_pool_mem_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "LARGE POOL MEM FREE = %d", stats->large_pool_mem_free, NULL);currentLine++;
    
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "TOTAL ALLOCS SLOTS = %d", stats->total_alloc_amount, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "TOTAL ALLOCS USED = %d", stats->total_alloc_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "TOTAL ALLOCS FREE = %d", stats->total_alloc_free, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "TOTAL MEMORY AMOUNT = %d", stats->total_memory_amount, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "TOTAL MEMORY USED = %d", stats->total_memory_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "TOTAL MEMORY FREE = %d", stats->total_memory_free, NULL);currentLine++;
    
    ASSERT(currentLine == nbLinesTotal, "WRONG AMOUNT OF LINES WRITTEN");
    
    *linesCount = nbLinesTotal;
    return statsLines;
}
