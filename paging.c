#include "paging.h"

extern void set_paging(uint32_t* pt);
extern void enablePaging();
extern void disablePaging();
// extern void invalidateEntry(uint32_t address);

void init_page_allocator()
{
    int res = pfm_setup_map(PFM_LOCATION);

    if(res != 0)
    {
        // Can't setup PFM, we're fucked.
    }

    struct page_table_info* kpt = pa_build_kernel_pagetable(KPT_LOCATION);

    if(kpt == NULL)
    {

    }
    
    // Start with 2 directories. One for the first 4MB that is identity-mapped
    // And another for the second 4MB segment where the KPT resides.
    pa_directory_load_at(kpt, 0, 1025 * PAGE_SIZE);
    pa_directory_load_at(kpt, 1, 1026 * PAGE_SIZE);
    
    // Identity-map the first 4MB. This will contain the BIOS 1MB, PFM 1 MB
    // text, data, rodata sections in 1MB with the last MB free but mapped.
    for(int i = 0; i < 1024; i++)
    {
        uint32_t addr = i * 0x1000;
        pa_pt_alloc_pageaddr_at(kpt, addr, addr);
    }

    // After the first 4MB have been reserved, allocate a backing page for the 
    // KPT. (KPT is a list of 1024 4 byte PDE entries).
    pa_pt_alloc_pageaddr(kpt, KPT_LOCATION);
    
    pa_set_current_pagetable(kpt);
    
    uint32_t ptr1 = 0;
    uint32_t ptr2 = 0;
    uint32_t ptr3 = 0;
    uint32_t ptr4 = 0;
    
    enablePaging();
    
    pa_pt_alloc_page(kpt, &ptr1);
    pa_pt_alloc_page(kpt, &ptr2);
    pa_pt_alloc_page(kpt, &ptr3);
    pa_pt_alloc_page(kpt, &ptr4);

    pa_print_kpt(kpt);
}

void pa_print_kpt(struct page_table_info* pt)
{
    kWriteLog("----------------------------------------");
    kWriteLog("Printing Kernel Page table");
    kWriteLog_format1d_stacksafe("KPT param stack address : %d", (uint32_t)&pt);
    kWriteLog_format1d_stacksafe("KPT Pointer : %d", (uint32_t)pt);

    int a = 0;
    int addr = 0;
    for(int k = 0; k < 1024; k++)
    {
        if(PD_PRESENT(pt->page_directory[k]))
        {
            kWriteLog_format1d_stacksafe("[PDE #%d]", k);
        }
        else
        {
            kWriteLog_format1d_stacksafe("PDE #%d EMPTY", k);
        }

        kWriteLog_format1d_stacksafe("Bits : %d", pt->page_directory[k]);

        if(PD_PRESENT(pt->page_directory[k]))
        {
            for(int i = 0; i < 1024; i++)
            {
                int ptIndex = i + (k * 1024);
                if(PT_PRESENT(pt->page_tables[ptIndex]))
                {
                    kWriteLog_format1d_stacksafe("  [PTE #%d] ***", ptIndex);
                }
                else
                {
                    kWriteLog_format1d_stacksafe("  [PTE #%d]", ptIndex);
                }

                kWriteLog_format1d_stacksafe("  Bits = %d", pt->page_tables[ptIndex]);
                kWriteLog_format1d_stacksafe("  Addr = %d", addr);

                addr += 0x1000;
            }
        }
        else
        {
            addr += 0x1000 * 1024;
            a += 1024;
        }
    }

    kWriteLog_format1d_stacksafe("Final PTE count = %d", a);
    kWriteLog_format1d_stacksafe("Final Addr = %d", addr);
    kWriteLog("Printing KPT done");
    kWriteLog("----------------------------------------");
}

void pa_test_paging()
{
    uint8_t* mybyte = (uint8_t*)(1024*1024*18);

    *mybyte = 0xFF;
}

struct page_table_info* pa_build_kernel_pagetable(uint32_t address)
{
    struct page_table_info* kpt = (struct page_table_info*)(address);

    int a = 0;
    int addr = 0;
    for(int k = 0; k < 1024; k++)
    {
        for(int i = 0; i < 1024; i++)
        {
            kpt->page_tables[a++] = 0;

            addr += 0x1000;
        }

        kpt->page_directory[k] = 0;
    }

    return kpt;
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

    uint32_t upper10 = addr & PAGEDIR_MASK;
    uint32_t pdeIndex = upper10 >> 22;

    // // Take the middle 10 bits to identify the page table (of the directory above)
    uint32_t lower10 = addr & PAGETAB_MASK;
    uint32_t pte = (lower10 >> 12) + (pdeIndex * 1024);

    // Checking if the pagetable is unmapped and not present.
    uint32_t ptBits = pt->page_tables[pte];
    if((ptBits & PAGEBITS) == 0)
    {
        int res = pfm_alloc_frame(physaddr);

        if(res < 0)
        {
            // Unable to reserve the frame.
            // TODO
            return;
        }

        pt->page_tables[pte] = (physaddr & PAGEBITS) | (PG_PRESENT | PG_WRITABLE);

        pa_invalidate_tlb(addr, physaddr);
    }
    else
    {
        // Cannot allocate, already allocated.
        // TODO : Inform.
        // TODO : What if current address is different from new address.
    }
}

void pa_directory_load(struct page_table_info* pt, uint32_t pdeIndex)
{
    uint32_t pde = pt->page_directory[pdeIndex];

    if(PD_PRESENT(pde) == FALSE)
    {
        // Find a free page frame and point the PDE to it
        // Then set it to present

        uint32_t frameAddr = 0;
        int res = pfm_find_free(&frameAddr);

        if(res == 0)
        {
            uint32_t vAddr = (uint32_t)&(pt->page_tables[1024 * pdeIndex]);
            pa_pt_alloc_pageaddr_at(pt, vAddr, frameAddr);
            
            //pfm_alloc_frame(frameAddr);
            pt->page_directory[pdeIndex] = frameAddr | 3;

            asm volatile("invlpg (%0)" ::"r" (frameAddr) : "memory");
        }
    }


    // TODO : Eventually going to have to check if the PDE is paged out
    // and load it.
}

void pa_directory_load_at(struct page_table_info* pt, uint32_t pdeIndex, uint32_t physAddr)
{
    uint32_t pde = pt->page_directory[pdeIndex];

    if(PD_PRESENT(pde) == FALSE)
    {
        uint32_t vAddr = (uint32_t)&(pt->page_tables[1024 * pdeIndex]);
        pa_pt_alloc_pageaddr_at(pt, vAddr, physAddr);

        pt->page_directory[pdeIndex] = physAddr | 3;

        asm volatile("invlpg (%0)" ::"r" (physAddr) : "memory");
    }
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

            if(pt->page_tables[pte] == 0)
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
    // TODO : MARK AS ALLOCATED
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
}

struct page_table_info* pa_get_current_pt()
{
    return currentPageTable;
}

void pa_decompose_vaddress(uint32_t vaddr, uint32_t* pde, uint32_t* pte, uint32_t* off)
{
    uint32_t upper10 = vaddr & PAGEDIR_MASK;
    uint32_t pdeIndex = upper10 >> 22;

    // Take the middle 10 bits to identify the page table (of the directory above)
    uint32_t lower10 = vaddr & PAGETAB_MASK;
    uint32_t pteIndex = (lower10 >> 12) + (pdeIndex * 1024);

    *pde = pdeIndex;
    *pte = pteIndex;
    *off = vaddr & PAGEOFF_MASK;
}

void pa_handle_pagefault(uint32_t addr, uint32_t code)
{
    (void)code;
    // Addr is the v address
    // Need to decompose into PDE and PTE to get correct entry
    uint32_t pde = 0;
    uint32_t pte = 0;
    uint32_t off = 0;
    pa_decompose_vaddress(addr, &pde, &pte, &off);

    Debugger();

    pa_directory_load(pa_get_current_pt(), pde);

    uint32_t entry = pa_get_current_pt()->page_tables[pte];

    if(entry == 0 && PA_LOAD_ON_DEMAND == TRUE)
    {
        pa_pt_alloc_pageaddr(pa_get_current_pt(), PAGE_ALIGN(addr));

        asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
    }
    else
    {
        // Crash the system, reference to unallocated page.
    }

    //pa_print_kpt(currentPageTable);
}

void pa_invalidate_tlb(uint32_t vaddr, uint32_t paddr)
{
    asm volatile("invlpg (%0)" ::"r" (vaddr) : "memory");
    asm volatile("invlpg (%0)" ::"r" (paddr) : "memory");
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
