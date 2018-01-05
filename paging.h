#ifndef PAGING_H
#define PAGING_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

#define PAGE_SIZE 4096

#define PAGEDIR_MASK 0xFFC00000
#define PAGETAB_MASK 0x3FF000
#define PAGEBITS (PAGEDIR_MASK | PAGETAB_MASK)
#define PAGEOFF_MASK 0xFFF

#define PAGE_ALL_PRESENT
#define PA_LOAD_ON_DEMAND 1

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

struct _page_directory
{
    uint32_t page_directory[1024] __attribute__((aligned(4096)));
} __attribute__((aligned(4096)));

struct _page_table
{
    uint32_t page_tables[1024*1024] __attribute__((aligned(4096)));
} __attribute__((aligned(4096)));

enum page_frame_flags
{
    PG_PRESENT = 1,
    PG_WRITABLE = 2,
    PG_ACCESSED = 32,
    PG_DIRTY = 64
};


// Storing a default page table before I start getting multiple address mappings
// TODO : Remove this & refs
struct page_table_info* defaultPageTable;

// Physical memory pages, this PT is not supposed to be loaded.
// Each entry is not a PTE but some bits to mark each physical page
// as occupied or other info.
// TODO : Changed to pointer, todo allocate somewhere
struct page_table_info* kernelPagetable;

#define KPT_LOCATION (1024 * PAGE_SIZE)

// Point to the page table currently loaded
struct page_table_info* currentPageTable;

// Paging Public interface

/**
 * Initialize the Page Allocation module. This will automatically
 * build and install a pagetable to the MMU and activate it. After
 * a call to this function, paging will be ONLINE.
 */
void init_page_allocator();

/**
 * Just a test function to allow debugging.
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
 * Allocate a range of pages. Range is [start, end)
 */
void pa_pt_alloc_pagerange(struct page_table_info* pt, uint32_t startAddress, uint32_t endAddress);

/**
 * Load a page directory on the current pagetable. This will allocate a new page
 * and point the directory to it, that page will contain the 1024 PTE.
 * If the directory is already loaded, this function will not do anything.
 */
void pa_directory_load(struct page_table_info* pt, uint32_t pdeIndex);

void pa_directory_load_at(struct page_table_info* pt, uint32_t pdeIndex, uint32_t physAddr);

/**
 * Find an unallocated page in the pagetable.
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
 * In the pagetable, map a virtual page to a physical page frame.
 */
void pa_map_page(struct page_table_info* pt, uint32_t paddr, uint32_t vaddr);

void pa_decompose_vaddress(uint32_t vaddr, uint32_t* pde, uint32_t* pte, uint32_t* off);

void pa_handle_pagefault(uint32_t addr, uint32_t code);

void pa_invalidate_tlb(uint32_t vaddr, uint32_t paddr);

void pa_print_kpt(struct page_table_info* pt);

// # Page Frame Functions #
#define PAGE_ALIGN(x) (x & 0xFFFFF000)
#define FRAME_INDX(x) (PAGE_ALIGN(x) / 4096)

struct page_frame_map
{
    uint8_t frames[1024*1024];

} __attribute__((aligned(4096)));

#define PFM_LOCATION (512 * PAGE_SIZE)

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

// OLD PAGING API
void setup_paging();
void test_paging();

void map_phys_address(uint32_t addressFrom, uint32_t addressTo);

// TODO : Find prefix for paging methods


int count_pages(enum page_frame_flags findFlags);
uint32_t* find_pages(enum page_frame_flags findFlags, int* count);



#endif
