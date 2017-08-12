#include "heap_memory.h"

/**
 * The heap manager manages two areas of memory. All this is for managing 
 * memory allocations. 
 *
 * The first block of memory is internal and used to store a big list of 
 * pre-allocated 'm_allocation' structures. These structures get initialized
 * when 'malloc' is called.
 *
 * The second block is the heap memory. There resides the blocks of data that 
 * are allocated by malloc. 
 *
 * Each 'm_allocation's are linked together sorted by start address of the 
 * memory. Each time a new allocation is initialized, it must be linked
 * at the right place in the linked list to keep the addresses sorted. This way
 * the space-searching algorithm can check each allocation together with the 
 * next to check if they have space in-between.
 *
 * One problem is that when deallocating memory, the 'm_allocation' struct stays
 * in memory but the fields are erased and the previous node is linked with the 
 * next after. This keeps the linked list in-order but creates holes in the 
 * first block of memory holding the 'm_allocation' structures. 
 *
 * This problem isn't going anywhere. Need to scan the entire list of allocation
 * everytime we need to get a new struct.
 *
*/


void init_memory_manager()
{
    uint32_t allocStartAddress = HEAP_ALLOCS_START;
    
    allocs = (struct m_allocation*)allocStartAddress;
    
    allocs_count = 0;
    
    for(size_t i = 0; i < HEAP_ALLOCS_AMOUNT; i++)
    {
        struct m_allocation* alloc = allocs + i;
        
        alloc->size = 0;
        alloc->p = NULL;
        alloc->allocated = FALSE;
        alloc->type = 0;
        alloc->flags = 0;
        
        alloc->previous = NULL;
        alloc->next = NULL;
    }
    
    struct m_allocation* stubFirstAlloc = allocs;
    stubFirstAlloc->size = 1;
    stubFirstAlloc->p = (void*)KERNEL_HEAP_START;
    stubFirstAlloc->allocated = TRUE;
    stubFirstAlloc->type = 0;
    stubFirstAlloc->flags = 0;
    stubFirstAlloc->next = NULL;
    stubFirstAlloc->previous = NULL;
    
    firstAlloc = stubFirstAlloc;
    lastAlloc = stubFirstAlloc;
}

void* kmalloc(uint32_t size)
{
    struct m_allocation* alloc = firstAlloc;
    uint32_t beginningSpace = (uint32_t)alloc->p - KERNEL_HEAP_START;
    if(beginningSpace >= size)
    {
        // Free space at the beginning of the heap
        
        struct m_allocation* newAlloc = mm_find_free_allocation();
        newAlloc->size = size;
        newAlloc->p = (void*)KERNEL_HEAP_START;
        newAlloc->allocated = TRUE;
        newAlloc->type = 0;
        newAlloc->flags = 0;
        
        // Assert that next's previous must be NULL?
        mm_link_allocs(newAlloc, firstAlloc);
        newAlloc->previous = NULL;

        firstAlloc = newAlloc;
        lastAlloc = newAlloc;
        
        return firstAlloc->p;
    }
    
    struct m_allocation* current = firstAlloc;
    while(current != NULL)
    {
        struct m_allocation* next = current->next;
        
        if(next != NULL)
        {
            if(mm_get_space(current, next) >= size)
            {
                struct m_allocation* newAlloc = mm_find_free_allocation();
                newAlloc->size = size;
                newAlloc->p = (void*)mm_data_tail(current);
                newAlloc->allocated = TRUE;
                newAlloc->type = 0;
                newAlloc->flags = 0;
                
                // Node linking
                mm_link_allocs(current, newAlloc);
                mm_link_allocs(newAlloc, next);
                
                lastAlloc = newAlloc;
                
                return newAlloc->p;
            }
            
        }
        else
        {
            uint32_t spaceToHeapEnd = (KERNEL_HEAP_START +  KERNEL_HEAP_LENGTH) - mm_data_tail(current);
            if(spaceToHeapEnd >= size)
            {
                struct m_allocation* newAlloc = mm_find_free_allocation();
                newAlloc->size = size;
                newAlloc->p = (void*)mm_data_tail(current);
                newAlloc->allocated = TRUE;
                newAlloc->type = 0;
                newAlloc->flags = 0;
                
                mm_link_allocs(current, newAlloc);
                
                lastAlloc = newAlloc;
                
                return newAlloc->p;
            }
            else
            {
                // No space for alloc on the heap
                Debugger();
                return NULL;
            }
        }
        
        current = next;
    }
    
    return NULL;
}

void kfree(void* ptr)
{
    struct m_allocation* current = lastAlloc;
    while(current != NULL)
    {
        if(current->p == ptr)
        {
            if(current->allocated == FALSE)
            {
                // TODO : Handle bad dealloc
                return;
            }
            
            memset(current->p, 0, current->size);
            
            mm_link_allocs(current->previous, current->next);
        }
        
        current = current->previous;
    }
}

uint32_t mm_get_space(struct m_allocation* first, struct m_allocation* second)
{
    return mm_data_head(second) - mm_data_tail(first);
}

uint32_t mm_data_head(struct m_allocation* target)
{
    return (uint32_t)target->p;
}

uint32_t mm_data_tail(struct m_allocation* target)
{
    return (uint32_t)target->p + target->size;
}

void mm_link_allocs(struct m_allocation* first, struct m_allocation* second)
{
    first->next = second;
    second->previous = first;
}

struct m_allocation* mm_find_free_allocation()
{
    for(size_t i = 0; i < HEAP_ALLOCS_AMOUNT; i++)
    {
        struct m_allocation* alloc = allocs + i;
        
        if(alloc->allocated == FALSE)
        {
            return alloc;
        }
    }
    
    return NULL;
}