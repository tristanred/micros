#include "heap_memory.h"

void init_memory_manager()
{
    alloc_vector = (struct m_allocation*)(1024 * 1024 * 5); // Place the initial alloc table at 5MB
    
    alloc_count = STARTING_ALLOC_COUNT;
    alloc_vector_total_size = sizeof(struct m_allocation) * STARTING_ALLOC_COUNT;
    
    for(size_t i = 0; i < alloc_count; i++)
    {
        struct m_allocation* target = alloc_vector + i;
        
        target = NULL;
    }
    
    
}

void* kmalloc(size_t size)
{
    struct m_allocation* found = mm_find_free_alloc_slot(size);
    
    return found->p;
}

struct m_allocation* mm_find_free_alloc_slot(size_t size)
{
    for(size_t i = 0; i < alloc_count; i++)
    {
        struct m_allocation* target = (alloc_vector + i);
        struct m_allocation* next = (alloc_vector + (i + 1));
        
        // TODO : First need to check if enough space exist for the entity
        if(target->allocated == FALSE)
        {
            target->size = size;
            return target;
        }
    }
    
    // Need to check if we can enlarge the alloc table
    
    return NULL;
}