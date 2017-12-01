#include "paging.h"

extern void set_paging(uint32_t* pt);
extern void enablePaging();
// extern void invalidateEntry(uint32_t address);

void init_page_allocator()
{
    
}

struct page_table_info* pa_create_pagetable()
{
    // TODO : Figure out where to put the PTI in memory
    // since we don't have any memory pages yet.
    // Put in kernel pagetable ?
    return NULL;
}


void pa_set_current_pagetable(struct page_table_info* pt)
{
    // Does not actually set the MMU pt used.
    currentPageTable = pt;
}

void pa_alloc_pages(size_t count)
{
    // Currently allocating only one page, at the end of the range of pages
    // currently allocated.
    
    struct page_table_info* pt = pa_get_current_pt();
    
    int p = 0;
    BOOL seek = TRUE;
    while(seek)
    {
        if(pt->page_tables[p++] & PG_PRESENT != PG_PRESENT)
        {
            // Flip the user/protect/present flags.
            // TODO : Find Physical memory location to place the page in
            uint32_t freeaddr = pa_find_free_physical_page();
            pa_map_page(freeaddr, p * PAGE_SIZE);
            
            seek = FALSE;
        }
    }
    
    // for(int k = 0; k < 1024; k++)
    // {
    //     for(int i = 0; i < 1024; i++)
    //     {
    //         int p = i + (k * 1024);
            
    //         if(pt->page_tables[p] & PG_PRESENT != PG_PRESENT)
    //         {
    //             // Flip the user/protect/present flags. Keep the address bits
    //             pt->page_tables[p] = pt->page_tables[p] | 3;
                
    //             return;
    //         }
    //     }
    // }
}

struct page_table_info* pa_get_current_pt()
{
    return currentPageTable;
}

uint32_t pa_find_free_physical_page()
{
    int p = 0;
    while(p < (1024*1024))
    {
        if(physicalPageMapping.page_tables[p] & PG_PRESENT != PG_PRESENT)
        {
            return p * PAGE_SIZE;
        }
        
        p++;;
    }
    
    // No free pages in memory.
    return 0; // TODO : return error instead
}

void pa_map_page(uint32_t paddr, uint32_t vaddr)
{
    // Take top 10 bits to identify the page directory
    uint32_t upper10 = vaddr & 0xFFC00000;
    uint32_t pdeIndex = upper10 >> 22;
    
    // Take the middle 10 bits to identify the page table (of the directory above)
    uint32_t lower10 = vaddr & 0x3FF000;
    uint32_t pte = (lower10 >> 12) + (pdeIndex * 1024);
    
    // Assign the 12 low bits from the target with the flags Present and R/W.
    pa_get_current_pt()->page_tables[pte] = (paddr & 0xFFFFF000) | 3;
    
    // Mark the physical mapping as used
    physicalPageMapping.page_tables[pte] = physicalPageMapping.page_tables[pte] | 3;
    
    // I'm invalidating both addresses just in case, will test for validity.
    asm volatile("invlpg (%0)" ::"r" (vaddr) : "memory");
    asm volatile("invlpg (%0)" ::"r" (paddr) : "memory");
}


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
    (void)findFlags;
    int totalCount = 0;
    // for(int i = 0; i < 1024*1024; i++)
    // {
    //     if(kMemoryManager->currentPageTable->page_tables[i] & findFlags)
    //         totalCount++;
    // }
    
    return totalCount;
}

uint32_t* find_pages(enum page_frame_flags findFlags, int* count)
{
    (void)findFlags;
    (void)count;
    
    return NULL;
}
