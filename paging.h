#ifndef PAGING_H
#define PAGING_H

/**
 * Page Allocator module
 * 
 * The Page Allocator is used to implement Virtual Memory. In a system with 
 * virtual memory, each memory address can be mapped to an arbitrary address in
 * physical RAM. This brings several advantages. One that the system can use 
 * any memory address it wants, we can use addresses in the 3GB range even on a 
 * system with 256MB of RAM. Secondly, we can protect memory ranges so that 
 * usermode programs cannot access memory owned by the kernel and other 
 * processes.
 * 
 * In Virtual Memory, the system memory is separated into pages. Each page is 
 * 4096 bytes in size. Pages are mapped to Page Frames. Pages make reference to 
 * a virtual block of memory and Page Frames are the actual location in physical
 * RAM. A page is accessed by its virtual address with the intent that the 
 * address will never change, this address is used by programs to access their 
 * data. The page is mapped to a Page Frame by manipulating an entry in the 
 * page table. The resulting address specifies where in physical ram the page 
 * will point to. By changing the entry, a memory address can point to different
 * blocks of data.
 * 
 * Each time a memory address is used (by dereferencing a pointer by example)
 * the processor will check the page table to see where to go in physical ram to
 * get the data. It has a few hops to go through so each memory access can be 
 * 2-3x slower than just hitting the address directly. To speed things up, each
 * time the processor finds the physical address it keeps a table of 
 * virtual->physical addresses so it doesn't have to redo the pagetable lookups.
 * 
 * A page table entry (PTE) contains the target physical address and 12 bits of
 * flags. Important flags include if the page is present, writable, accessible 
 * by normal users and others. Pages can be marked as non-present, meaning that
 * the data has been moved somewhere else (ex: to disk) or has not been 
 * allocated yet. When the processor hits a PTE with the PRESENT flag (bit 0)
 * set to 0 it will launch a PageFault exception with an interrupt. The kernel
 * will receive the interrupt and the target address + error code, It then has 
 * to work to read the data back from disk, set it somewhere in physical RAM 
 * (anywhere) and link the virtual to the physical address in the PTE bits. When
 * the interrupt returns, the faulting instruction will be re-ran but this time
 * the page will have been marked as present and the memory can be accessed.
 * 
 * All the PTE entries are contained in a Page Directory (PDE for Page Directory
 * Entry, all PDE are in the root pagetable structure) there are 1024 PDE so 
 * each of them can oversee 1024 pages so 4 MB. Currently each PDE need to map
 * to sequential 4MB segments. This is due to my pagetable structure being all
 * in one struct. This avoids having to disable paging when handling a lookup
 * due to having to read the PDE bytes directly in physical memory.
 * 
 * All this enables us to keep separate memory mappings for each process. This 
 * way, when a process tries to access another process's memory the pages will 
 * not be mapped and a PageFault will trigger. We can stop the access and kill 
 * the faulting process. We can even give two processes access to the same 
 * virtual address but pointing to a different page frame in RAM so they will
 * have different data.
 * 
 * This Page Allocator initializes a kernel pagetable by mapping the first 4MB
 * of data to the same physical addresses. This is done mostly for the BIOS 
 * structures contained in the first 1MB of memory. 
 * 
 * The second MB is used to contain the kernel code stack and 
 * initialized/uninitialized data segments.
 * 
 * The third MB contains the Page Frame Map (PFM). The PFM tracks the physical
 * memory allocations. We can flag each 4KB segments to track if it is used
 * or free. Only the kernel has access to this structure and is shared between
 * all pagetables.
 * 
 * The fourth MB doesn't have anything right now. It is probable that the PFM
 * will grow to 2MB because it is currently using 8bit entries and more might 
 * be needed.
 * 
 * The kernel page table is located on the first page of the 5th MB followed by
 * the first 2 page directories backing pages.
 * 
 */

#include <stddef.h>
#include <stdint.h>

#include "types.h"
#include "kernel.h"
#include "memory_zones.h"

#define PAGE_SIZE 4096

#define PAGEDIR_MASK 0xFFC00000
#define PAGETAB_MASK 0x3FF000
#define PAGEBITS (PAGEDIR_MASK | PAGETAB_MASK)
#define PAGEOFF_MASK 0xFFF

#define PAGE_ALL_PRESENT
//#define PA_LOAD_ON_DEMAND 1
#define PA_LOAD_ON_DEMAND 0

// PDE Flags
#define PD_PRESENT(x)   ((x & 0x1)  == 0x1)
#define PD_RW(x)        ((x & 0x2)  == 0x2)
#define PD_SUPER(x)     ((x & 0x4)  == 0x4)
#define PD_PWT(x)       ((x & 0x8)  == 0x8)
#define PD_PCD(x)       ((x & 0x10) == 0x10)
#define PD_ACCES(x)     ((x & 0x20) == 0x20)
#define PD_ADDRBITS(x)  (x & 0xFFFFF000)
#define PD_FLAGBITS(x)  (x & 0xFFF)

// PTE Flags
#define PT_PRESENT(x)   ((x & 0x1) == 0x1)
#define PT_RW(x)        ((x & 0x2) == 0x2)
#define PT_SUPER(x)     ((x & 0x4) == 0x4)
#define PT_PWT(x)       ((x & 0x8) == 0x8)
#define PT_PCD(x)       ((x & 0x10) == 0x10)
#define PT_ACCES(x)     ((x & 0x20) == 0x20)
#define PT_DIRTY(x)     ((x & 0x40) == 0x40)
#define PT_ADDRBITS(x)  (x & 0xFFFFF000)
#define PT_FLAGBITS(x)  (x & 0xFFF)

struct page_table_info
{
    uint32_t page_directory[1024] __attribute__((aligned(4096)));
    uint32_t page_tables[1024*1024] __attribute__((aligned(4096)));
} __attribute__((aligned(4096)));

// General structure that can be used to represent a range of bytes in a page.
// Not used for the moment.
struct page
{
    uint8_t data[4096] __attribute__((aligned(4096)));
}__attribute__((aligned(4096)));

// Flags of a PDE. These are organized by CPU architecture so refer to the
// Intel - System Programming Guide guide for the other values.
enum page_frame_flags
{
    PG_PRESENT = 1,
    PG_WRITABLE = 2,
    PG_ACCESSED = 32,
    PG_DIRTY = 64
};

struct page_allocator_module
{
    // Page table currently loaded in CR3
    struct page_table_info* current_pt;
    
    // Kernel page table
    struct page_table_info* kernelPagetable;
    
    // Limits in page number of where the Page Allocator can allocate memory
    // from. This range is checked in pa_pt_alloc_page where we don't care
    // where the memory is from, it will always be between min and max.
    uint32_t min_page_alloc;
    uint32_t max_page_alloc;
};
struct page_allocator_module* pa_module;

// Point to the page table currently loaded
struct page_table_info* currentPageTable;

// Paging Public interface

/**
 * Initialize the Page Allocation module. This will automatically
 * build and install a pagetable to the MMU and activate it. After
 * a call to this function, paging will be ONLINE.
 */
void init_page_allocator(struct kernel_info_block* kinfo);

/**
 * Just a test function to reference an unloaded page. If the system is 
 * allocating pages freely it won't crash the system but if not and the page
 * was not allocated beforehand it will crash.
 */
void pa_test_paging();

/**
 * Create a pagetable and set the pages for supervisor access.
 * All pages are identity mapped and the Page Directories are unmapped.
 */
struct page_table_info* pa_build_kernel_pagetable(uint32_t address);

/**
 * Create a blank pagetable with user access and all pages and directories are
 * unmapped.
 */
struct page_table_info* pa_create_pagetable();

/**
 * Allocate a page from anywhere in virtual memory. Returns the address of the
 * page in the 'addr' parameter.
 * 
 * Not meant to be used a whole lot because the page can be anywhere in memory
 * and we can't be sure that if we allocate two pages they will be contiguous
 * in virtual memory. Used if we only need 4096 bytes. Overflows will not be 
 * detected.
 */
void pa_pt_alloc_page(struct page_table_info* pt, uint32_t* addr);

/**
 * Allocate a page at the 4kb aligned address 'addr'.
 */
void pa_pt_alloc_pageaddr(struct page_table_info* pt, uint32_t addr);

/**
 * Allocate a page at the specified virtual address backed in RAM by a pageframe
 * at the specified physical address.
 */
void pa_pt_alloc_pageaddr_at(struct page_table_info* pt, uint32_t addr, uint32_t physaddr);

/**
 * Load a page directory on the current pagetable. This will allocate a new page
 * and point the directory to it, that page will contain the 1024 PTE.
 * If the directory is already loaded, this function will not do anything.
 */
void pa_directory_load(struct page_table_info* pt, uint32_t pdeIndex);

/**
 * Load a page directory using a specified page frame. This will allocate a new
 * page and point the directory to it, that page will contain the 1024 PTE.
 * If the directory is already loaded, this function will not do anything.
 */
void pa_directory_load_at(struct page_table_info* pt, uint32_t pdeIndex, uint32_t physAddr);

/**
 * Find an unallocated page in the pagetable.
 * Returns the 4kb aligned address of the page.
 */
uint32_t pa_pt_find_free_page(struct page_table_info* pt);

/**
 * Set the current pagetable as active on the processor.
 * TODO : Check the fuck out of the pt parameter. OS will
 * crash if we try to setup a bad pagetable.
 */
void pa_set_current_pagetable(struct page_table_info* pt);

/**
 * Get a pointer to the current pagetable.
 */
struct page_table_info* pa_get_current_pt();

/**
 * Using the specified pagetable, map the given page in physical ram at a free
 * page frame. The physical page frame is found by scanning a free frame in RAM.
 */
void pa_alloc_map_page(struct page_table_info* pt, uint32_t page);

/**
 * Extract the information from a virtual address. A virtual address is composed
 * of 3 information. 
 * First 10 bits are the index of the PDE.
 * Second 10 bits are the index of the PTE inside the PDE of the first 10 bits.
 * Last 12 bits are the offset into a page of size 4096 bytes.
 */
void pa_decompose_vaddress(uint32_t vaddr, uint32_t* pde, uint32_t* pte, uint32_t* off);

/**
 * Function called when a pagefault is raised. Currently does not crash the 
 * system if we reference a page that was not allocated. The faulting
 * instruction will be repeated indefinitely.
 */
void pa_handle_pagefault(uint32_t addr, uint32_t code);

/**
 * Invalidate the processor TLB cache.
 */
void pa_invalidate_tlb(uint32_t vaddr, uint32_t paddr);

/**
 * Print the kernel page table to the serial OUT port. Used for debugging.
 */
void pa_print_kpt(struct page_table_info* pt);

// # Page Frame Functions #
#define PAGE_ALIGN(x) (x & 0xFFFFF000)
#define FRAME_INDX(x) (PAGE_ALIGN(x) / 4096)

// Meant to occupy one full page.
struct page_frame_map
{
    uint8_t frames[1024*1024];

} __attribute__((aligned(4096)));

enum physical_frame_flags
{
    PFM_ALLOCATED = 1,
    PFM_WRITABLE = 2
};

struct page_frame_map* pfm_frame_map;

/**
 * Mark the page frame at 'addr' as allocated.
 */
int pfm_alloc_frame(uint32_t addr);

/**
 * Mark the page frame at 'addr' as free.
 */
int pfm_free_frame(uint32_t addr);

/**
 * Copy the data from one frame to the other.
 */
int pfm_copy_frame(uint32_t fromAddr, uint32_t toAddr);

/**
 * Find a free page frame in physical memory.
 */
int pfm_find_free(uint32_t* frame);

/**
 * Find a list of free page frames.
 */
int pfm_find_list(uint32_t amount, uint32_t** list);

// # Private Page Frame functions #
/**
 * Setup the page frame bitmap.
 */
int pfm_setup_map(uint32_t addr);

/**
 * Mark the frames of physical memory that we have on the system.
 * This is the amount of available RAM.
 */
int pfm_set_avail_frames();

#endif
