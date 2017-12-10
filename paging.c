#include "paging.h"

extern void set_paging(uint32_t* pt);
extern void enablePaging();
// extern void invalidateEntry(uint32_t address);

void init_page_allocator()
{
    struct page_table_info* kpt = pa_build_kernel_pagetable();
    
    pa_set_current_pagetable(kpt);
}

struct page_table_info* pa_build_kernel_pagetable()
{
    struct page_table_info* kpt = (struct page_table_info*)(512*4096);
    
    int a = 0;
    int addr = 0;
    for(int k = 0; k < 1024; k++)
    {
        for(int i = 0; i < 1024; i++)
        {
            kpt->page_tables[a++] = addr | 3;
            
            addr += 0x1000;
        }
        
        // Map all PDE to a different PTE set (each PDE maps 4MB)
        //kpt->page_directory[k] = ((uint32_t)&kpt->page_tables[k * 1024]) | 3;
        
        // Each PDE points to the first pagetable (first 4MB).
        kpt->page_directory[k] = 3; // super, rw, present, no address
    }
    
    Debugger();
    
    // Manually set the 2 first PDE to the first pagetable.
    // So 0x400000 and 0x0 points to the first byte.
    kpt->page_directory[0] = (uint32_t)(&(kpt->page_tables[0])) | 3; // same as (uint32_t)(257*4096)
    kpt->page_directory[1] = (uint32_t)(&(kpt->page_tables[0])) | 3; // same as (uint32_t)(257*4096)
    
    int* oneData = (int*)0xF00;
    int* twoData = (int*)0x400F00;
    
    *oneData = 0xFEFE;
    *twoData = 0xBABA;
    
    ASSERT(*oneData != *twoData, "Paging does not work.");
    
    // set_paging(kpt); // Un comment to test paging immediately
    // enablePaging();
    
    ASSERT(*oneData == *twoData, "Paging does not work.");
    
    return kpt;
    //kernelPagetable = kpt;
}

struct page_table_info* pa_create_pagetable()
{
    // TODO : Figure out where to put the PTI in memory
    // since we don't have any memory pages yet.
    // Put in kernel pagetable ?
    return NULL;
}

void pa_pt_alloc_page(struct page_table_info* pt, uint32_t* addr)
{
    uint32_t allocAddress = pa_pt_find_free_page(pt);
    
    pa_pt_alloc_pageaddr(pt, allocAddress);
    
    *addr = allocAddress;
}

void pa_pt_alloc_pageaddr(struct page_table_info* pt, uint32_t addr)
{
    
    uint32_t frameAddress = 0;
    int res = pfm_find_free(&frameAddress);
    
    if(res < 0)
    {
        // No free memory in RAM.
        // TODO
        return;
    }
    
    pa_pt_alloc_pageaddr_at(pt, addr, frameAddress);
}

void pa_pt_alloc_pageaddr_at(struct page_table_info* pt, uint32_t addr, uint32_t physaddr)
{
    /* To allocate the page containing addr, we must first find a free page 
     * frame in physical memory. Also must check if the page is not already
     * mapped.
     */

    uint32_t upper10 = addr & 0xFFC00000;
    uint32_t pdeIndex = upper10 >> 22;
    
    // // Take the middle 10 bits to identify the page table (of the directory above)
    uint32_t lower10 = addr & 0x3FF000;
    uint32_t pte = (lower10 >> 12) + (pdeIndex * 1024);
    
    // Checking if the pagetable is unmapped and not present.
    if((pt->page_tables[pte] & 0xFFFFF000) == 0)
    {
        int res = pfm_alloc_frame(physaddr);
        
        if(res < 0)
        {
            // Unable to reserve the frame.
            // TODO
            return;
        }
        
        pt->page_tables[pte] = (physaddr & 0xFFFFF000) | (PG_PRESENT | PG_WRITABLE);
    }
    else
    {
        // Cannot allocate, already allocated. 
        // TODO : Inform.
        // TODO : What if current address is different from new address.
    }
}

void pa_pt_alloc_pagerange(struct page_table_info* pt, uint32_t startAddress, uint32_t endAddress)
{
    (void)pt;
    (void)startAddress;
    (void)endAddress;
}

uint32_t pa_pt_find_free_page(struct page_table_info* pt)
{
    /*
        Finding a free page : 
        Iterate through the page directory entries
        If a directory is present, check all pagetables entries for one that is
        not 'present', if theres one we can allocate there.
        If we get to the end of the directory, check the next directory.
        If we get to the end of the pagetables, we need to open up a new 
        directory.
        
        
        
    */
    
    for(int i = 0; i < 1024; i++)
    {
        int pde = pt->page_directory[i];
        
        if((pde & 0x1) == 0) // If not present
        {
            continue;
        }
        
        for(int k = 0; k < 1024; k++)
        {
            int pte = (i * 1024) + k;
            
            if((pt->page_tables[pte] & 0xFFFFF000) == 0)
            {
                // Found a free page ! Woohoo
                return pte * PAGE_SIZE;
            }
        }
    }
    
    // Need to allocate a new PDE to point to a new page of entries
    // Means we need to allocate a new page and have our new PDE point to it
    
    // Find a free page frame
    uint32_t freeFrame = 0;
    int res = pfm_find_free(&freeFrame);
    
    if(res < 0)
    {
        // TODO : Out of memory
        return 0;
    }
    
    // Find a free PDE 
    
    BOOL found = FALSE;
    uint32_t index = 0;
    for(int i = 0; i < 1024; i++)
    {
        uint32_t pde = pt->page_directory[i];
        
        if(pde == 0)
        {
            found = TRUE;
            index = i;
            break;
        }
    }
    
    if(found == FALSE)
    {
        // No free PDE, wow, quite unlucky !
    }
    
    // Need to find a space in the virtual address space
    uint32_t pdeFirstPageAddr = 1024 * PAGE_SIZE * index;
    
    pt->page_directory[index] = pdeFirstPageAddr | 3;
    
    // Now the page directory entry is loaded and points to a page that will
    // contain all the page table entries. The first of which is free and can be
    // returned from this function.
    
    return pdeFirstPageAddr;
}

void pa_set_current_pagetable(struct page_table_info* pt)
{
    currentPageTable = pt;
    
    set_paging(pt->page_directory);
    enablePaging();
}

struct page_table_info* pa_get_current_pt()
{
    // Can also read CR3 register
    return currentPageTable;
}

void pa_map_page(struct page_table_info* pt, uint32_t paddr, uint32_t vaddr)
{
    // Take top 10 bits to identify the page directory
    uint32_t upper10 = vaddr & 0xFFC00000;
    uint32_t pdeIndex = upper10 >> 22;
    
    // Take the middle 10 bits to identify the page table (of the directory above)
    uint32_t lower10 = vaddr & 0x3FF000;
    uint32_t pte = (lower10 >> 12) + (pdeIndex * 1024);
    
    // Assign the 12 low bits from the target with the flags Present and R/W.
    pt->page_tables[pte] = (paddr & 0xFFFFF000) | 3;
    
    // Mark the physical mapping as used
    pt->page_tables[pte] = pt->page_tables[pte] | 3;
    
    // I'm invalidating both addresses just in case, will test for validity.
    asm volatile("invlpg (%0)" ::"r" (vaddr) : "memory");
    asm volatile("invlpg (%0)" ::"r" (paddr) : "memory");
}

void pa_handle_pagefault()
{
    
}

int pfm_alloc_frame(uint32_t addr)
{
    uint32_t a = PAGE_ALIGN(addr);
    uint32_t pfmindex = a / 4096;
    
    pfm_frame_map->frames[pfmindex] |= (PFM_ALLOCATED | PFM_WRITABLE);
    
    return 0;
}

int pfm_free_frame(uint32_t addr)
{
    uint32_t a = PAGE_ALIGN(addr);
    uint32_t pfmindex = a / 4096;
    
    pfm_frame_map->frames[pfmindex] = 0;
    
    return 0;
}

int pfm_copy_frame(uint32_t fromAddr, uint32_t toAddr)
{
    uint32_t alignedFrom = PAGE_ALIGN(fromAddr);
    uint32_t alignedTo = PAGE_ALIGN(toAddr);
    
    for(size_t i = 0; i < PAGE_SIZE; i++)
    {
        uint8_t* byteFrom = (uint8_t*)alignedFrom + i;
        uint8_t* byteTo = (uint8_t*)alignedTo + i;
        
        *byteTo = *byteFrom;
    }
    
    return 0;
}

int pfm_find_free(uint32_t* frame)
{
    for(size_t i = 0; i < 1024*1024; i++)
    {
        if((pfm_frame_map->frames[i] & PFM_ALLOCATED) == pfm_frame_map->frames[i])
        {
            *frame = i * PAGE_SIZE;
            
            return 0;
        }
    }

    return -1;
}

int pfm_find_list(uint32_t amount, uint32_t** list)
{
    for(size_t i = 0; i < amount; i++)
    {
        uint32_t addr = 0;
        int res = pfm_find_free(&addr);
        
        if(res < 0)
        {
            // If we can't get all the required pages, return failure.
            return -1;
        }
        else
        {
            *list[i] = addr;
        }
    }

    return 0;
}

int pfm_setup_map(uint32_t addr)
{
    pfm_frame_map = (struct page_frame_map*)addr;
    
    for(size_t i = 0; i < 1024*1024; i++)
    {
        pfm_frame_map->frames[i] = 0;
    }
    
    return 0;
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
            defaultPageTable->page_tables[a++] = addr | 3; // attributes: supervisor level, read/write, present.
            
            addr += 0x1000; // Target the next 4KB page.
        }
        
        #ifdef PAGE_ALL_PRESENT
        // Currently for debugging, we'll identity-map all the pages to the 
        // physical address.
        
        // attributes: supervisor level, read/write, present
        defaultPageTable->page_directory[k] = ((uint32_t)&defaultPageTable->page_tables[k * 1024]) | 3;
        
        #else
        
        // If not, we must map the first 8MB (first 2 page directories) to
        // be present because most of the OS currently lives under the 8 first
        // MB's. Rest of the pages are marked not-present to allow testing
        // page faults.
        
        if(k <= 1)
        {
            page_directory[k] = ((uint32_t)&defaultPageTable->page_tables[k * 1024]) | 3;
        }
        else
        {
            page_directory[k] = ((uint32_t)&defaultPageTable->page_tables[k * 1024]) | 2;
        }

        #endif
        
    }
    
    set_paging(defaultPageTable->page_directory);
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
    defaultPageTable->page_tables[pte] = (addressTo & 0xFFFFF000) | 3;
    
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
