#include "memory.h"

#include "kernel_log.h"

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
 * everytime we need to get a new struct. One things that is not so bad is that
 * checking the entire list serially will benefit from cache locality and branch
 * predition.
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
        
        #ifdef MM_ENABLE_HEAP_ALLOC_CANARY
        mm_set_alloc_canary(newAlloc);
        #endif
        
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
                
                #ifdef MM_ENABLE_HEAP_ALLOC_CANARY
                mm_set_alloc_canary(newAlloc);
                #endif
                
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
                
                #ifdef MM_ENABLE_HEAP_ALLOC_CANARY
                mm_set_alloc_canary(newAlloc);
                #endif
                
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
                Debugger();
                return;
            }
            
            #ifdef MM_ENABLE_HEAP_ALLOC_CANARY
            
            if(mm_verify_alloc_canary(current) == FALSE)
            {
                kWriteLog_format1d("Overflow detected at address %d", (uint32_t)current->p);
                
                Debugger();
            }
            
            #endif
    
            
            memset(current->p, 0, current->size);
            
            mm_link_allocs(current->previous, current->next);
            
            // Keeping the other attributes of the allocation to maybe help with
            // debugging freed allocations.
            current->allocated = FALSE;
            current->next = NULL;
            current->previous = NULL;
            
            return;
        }
        
        current = current->previous;
    }
}

void kmzero(void* ptr)
{
    char* ptrFromChar = (char*)ptr;
    
    size_t fromSize = sizeof(*ptr);
    
    for(size_t i = 0; i < fromSize; i++)
    {
        ptrFromChar[i] = 0;
    }
}

void* kmemcpy( void *dest, const void *src, uint32_t count )
{
    uint8_t* ptrSrc = (uint8_t*)src;
    uint8_t* ptrDest = (uint8_t*)dest;
    
    for(size_t i = 0; i < count; i++)
    {
        ptrDest[i] = ptrSrc[i];
    }
    
    return dest;
}

void kmemplace(void* dest, uint32_t offset, const char* data, size_t count)
{
    char* cDest = (char*)dest;
    
    for(size_t i = 0; i < count; i++)
    {
        cDest[i + offset] = data[i];
    }
}

void kmemget(void* src, char* dest, uint32_t offset, size_t count, size_t* readSize)
{
    char* cSrc = (char*)src;
    
    size_t dataLength = 0;
    
    for(size_t i = 0; i < count; i++)
    {
        dest[i] = cSrc[i + offset];
        dataLength++;
    }
    
    *readSize = dataLength;
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

#ifdef MM_ENABLE_HEAP_ALLOC_CANARY

void mm_set_alloc_canary(struct m_allocation* alloc)
{
    alloc->size += MM_HEAP_ALLOC_CANARY_SIZE;
    
    kmemplace(alloc->p, alloc->size - MM_HEAP_ALLOC_CANARY_SIZE, MM_HEAP_ALLOC_CANARY_VALUE, MM_HEAP_ALLOC_CANARY_SIZE);
}

/**
 * Returns true if the canary was untouched, so no buffer overflow.
*/
BOOL mm_verify_alloc_canary(struct m_allocation* alloc)
{
    size_t dataLength = 0;
    char data[MM_HEAP_ALLOC_CANARY_SIZE];
    kmemget(alloc->p, data, alloc->size - MM_HEAP_ALLOC_CANARY_SIZE, MM_HEAP_ALLOC_CANARY_SIZE, &dataLength);
    
    if(strncmp(data, MM_HEAP_ALLOC_CANARY_VALUE, MM_HEAP_ALLOC_CANARY_SIZE) == 0)
    {
        return TRUE;
    }
    
    return FALSE;
}

BOOL mm_verify_all_allocs_canary()
{
    struct m_allocation* current = firstAlloc;
    while(current != NULL)
    {
        if(mm_verify_alloc_canary(current) == FALSE)
        {
            return FALSE;
        }
        
        current = current->next;
    }
    
    return FALSE;
}

#endif
