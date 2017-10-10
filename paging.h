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
 
// Storing a default page table before I start getting multiple address mappings
struct page_table_info defaultPageTable;

void init_page_allocator();

void pa_create_pagetable(struct page_table_info* pt);

void pa_set_current_pagetable(struct page_table_info* pt);

void

// OLD PAGING API
void setup_paging();
void test_paging();

void map_phys_address(uint32_t addressFrom, uint32_t addressTo);

// TODO : Find prefix for paging methods

enum page_frame_flags {
    PG_PRESENT = 1,
    PG_WRITABLE = 2,
    PG_ACCESSED = 32,
    PG_DIRTY = 64
};

int count_pages(enum page_frame_flags findFlags);
uint32_t* find_pages(enum page_frame_flags findFlags, int* count);



#endif
