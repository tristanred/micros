#include "heap_memory.h"

void init_memory_manager()
{
    kernel_heap_start = 1024 * 1024 * 5;
    kernel_heap_size = mm_find_max_memory() - kernel_heap_start;
    
    mm_setup_first_alloc_block();
    
}

void* kmalloc(uint32_t size)
{
    if(size == 0)
        return NULL;
        
    uint32_t freeSpace = mm_find_free_space(size);
    
    if(freeSpace != 0)
    {
        
    }
    
    return NULL;
}

uint32_t mm_find_max_memory()
{
    return 1024 * 1024 * 30; // Blah, let's just say 30MB for now.
}

void mm_setup_first_alloc_block()
{
    struct m_allocation* first_block = (struct m_allocation*)kernel_heap_start;
    
    first_block->size = sizeof(struct m_allocation) * STARTING_ALLOCS_COUNT;
    first_block->p = (void*)kernel_heap_start;
    first_block->allocated = TRUE;
    first_block->type = MEM_ALLOCS_BLOCK;
    first_block->flags = 0;
    first_block->next = NULL;
    
    first_alloc = first_block;
    last_alloc = first_block;
    
    alloc_count = 1;
}

uint32_t mm_find_free_space(uint32_t sizeNeeded)
{
    struct m_allocation* current = first_alloc;
    
    while(current != NULL)
    {
        struct m_allocation* next = current->next;
        
        if(next != NULL)
        {
            if(mm_get_space_between(current, next) >= sizeNeeded)
            {
                return (uint32_t)current->p + current->size;
            }
        }
        else
        {
            return (uint32_t)current->p + current->size;
        }
        
        current = current->next;
    }
    
    return 0;
}

uint32_t mm_get_space_between(struct m_allocation* first, struct m_allocation* second)
{
    return second->p - (first->p + first->size);
}

void* mm_create_alloc(uint32_t startAddress, uint32_t size)
{
    
}