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

enum page_frame_flags {
    PG_PRESENT = 1,
    PG_WRITABLE = 2,
    PG_ACCESSED = 32,
    PG_DIRTY = 64
};

// Storing a default page table before I start getting multiple address mappings
struct page_table_info defaultPageTable;

// Physical memory pages, this PT is not supposed to be loaded.
// Each entry is not a PTE but some bits to mark each physical page
// as occupied or other info.
struct page_table_info kernelPagetable;

// Point to the page table currently loaded, 
struct page_table_info* currentPageTable;

// Paging Public interface
void init_page_allocator();

struct page_table_info* pa_create_pagetable();

void pa_set_current_pagetable(struct page_table_info* pt);

void pa_alloc_pages(size_t count);

void pa_free_page(uint32_t page);

void pa_zero_write_page(uint32_t page);

void pa_map_page(uint32_t addressFrom, uint32_t addressTo);

size_t pa_count_present_pages();

// Paging Private interface
struct page_table_info* pa_get_current_pt();

uint32_t pa_find_free_physical_page();
void pa_map_page(uint32_t paddr, uint32_t vaddr);

// OLD PAGING API
void setup_paging();
void test_paging();

void map_phys_address(uint32_t addressFrom, uint32_t addressTo);

// TODO : Find prefix for paging methods


int count_pages(enum page_frame_flags findFlags);
uint32_t* find_pages(enum page_frame_flags findFlags, int* count);



#endif
