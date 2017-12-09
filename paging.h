#ifndef PAGING_H
#define PAGING_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

#define PAGE_SIZE 4096

#define PAGE_ALL_PRESENT

struct page_table_info
{
    uint32_t page_directory[1024] __attribute__((aligned(4096)));
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
 * Create a pagetable and set the pages for supervisor access.
 * All pages are identity mapped and the Page Directories are unmapped.
 */
struct page_table_info* pa_build_kernel_pagetable();

/**
 * Create a blank pagetable with user access and all pages and directories are
 * unmapped.
 */
struct page_table_info* pa_create_pagetable();

/**
 * Allocate a page at the 4kb aligned address 'addr'.
 */
void pa_pt_alloc_pageaddr(struct page_table_info* pt, uint32_t addr);

/**
 * Allocate a range of pages. Range is [start, end)
 */
void pa_pt_alloc_pagerange(struct page_table_info* pt, uint32_t startAddress, uint32_t endAddress);

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
 * Find a free page in the pagetable. Does not modify the page.
 */
uint32_t pa_find_free_physical_page(struct page_table_info* pt);

/**
 * Using the specified pagetable, map the given page in physical ram at a free
 * page frame. The physical page frame is found by scanning a free frame in RAM.
 */
void pa_alloc_map_page(struct page_table_info* pt, uint32_t page);

/**
 * In the pagetable, map a virtual page to a physical page frame.
 */
void pa_map_page(struct page_table_info* pt, uint32_t paddr, uint32_t vaddr);

void pa_handle_pagefault();

// OLD PAGING API
void setup_paging();
void test_paging();

void map_phys_address(uint32_t addressFrom, uint32_t addressTo);

// TODO : Find prefix for paging methods


int count_pages(enum page_frame_flags findFlags);
uint32_t* find_pages(enum page_frame_flags findFlags, int* count);



#endif
