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
 * When malloc is called, a new entry 'm_allocation' is registered and setup
 * with the values needed to manage the heap memory block.
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
 * everytime we need to get a new struct. One thing that is not so bad is that
 * checking the entire list serially will benefit from cache locality and branch
 * prediction.
 *
*/

/*
 * Module init function.
 *
 * Must be called before any memory calls are done. 
 * This creates the two areas, the allocation block and the heap.
 *
 * There is an allocation of 1 byte at first in order to init the firstAlloc and
 * lastAlloc to a value. This avoids having to check for NULL values in those at
 * every calls to malloc. Instead of doing this, we could just assign that first
 * allocation to something that will remain for the entire duration of the 
 * kernel.
 */
void init_memory_manager()
{

}

/**
 * Allocate memory on the heap.
 *
 * Allocations can be configured to set 'canary bytes' at the end of the memory
 * area to detect buffer overflows.
 */
void* kmalloc(uint32_t size)
{
    return kmallocf(size, MEM_NOFLAGS);
}

/**
 * Release memory allocated by malloc.
 */
void kfree(void* ptr)
{
    kfreef(ptr);
}

/**
 * Copy data from one pointer to another. Must specify a size to copy.
 * Don't put a wrong size. Seriously, do not.
 * TODO : Validate size of dest and src in the alloc list.
 */
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

/**
 * Same as kmalloc but with upgraded functionalities. The current kmalloc calls
 * are meant to be migrated over to this function eventually.
 */
void* kmallocf(uint32_t size, enum mm_alloc_flags f)
{
    uint32_t allocTotalSize = size;
    if((f | MEM_CHECKED) == f)
        allocTotalSize += MM_HEAP_ALLOC_CANARY_SIZE;
    
    if(firstAlloc == NULL)
    {
        struct m_allocation* newAlloc = (struct m_allocation*)KERNEL_HEAP_START;
        newAlloc->size = allocTotalSize;
        newAlloc->p = (void*)(newAlloc + sizeof(struct m_allocation));
        newAlloc->allocated = TRUE;
        newAlloc->type = MEM_ALLOC;
        newAlloc->flags = f;

        if((f | MEM_CHECKED) == f)
        {
            mm_set_alloc_canary(newAlloc);
        }

        firstAlloc = newAlloc;
        lastAlloc = newAlloc;
        
        return newAlloc->p;
    }
    
    struct m_allocation* current = firstAlloc;
    while(current != NULL)
    {
        struct m_allocation* next = current->next;
        
        if(next != NULL)
        {
            if(mm_get_space(current, next) >= allocTotalSize + sizeof(struct m_allocation))
            {
                struct m_allocation* newAlloc = (struct m_allocation*)mm_data_tail(current);
                newAlloc->size = allocTotalSize;
                newAlloc->p = (void*)(newAlloc + sizeof(struct m_allocation));
                newAlloc->allocated = TRUE;
                newAlloc->type = MEM_ALLOC;
                newAlloc->flags = f;
                
                if((f | MEM_CHECKED) == f)
                {
                    mm_set_alloc_canary(newAlloc);
                }
                
                // Node linking
                mm_link_allocs(current, newAlloc);
                mm_link_allocs(newAlloc, next);
                
                lastAlloc = newAlloc;
                
                return newAlloc->p;
            }
        }
        else
        {
            uint32_t spaceToHeapEnd = mm_space_to_end(current);
            if(spaceToHeapEnd >= allocTotalSize)
            {
                struct m_allocation* newAlloc = (struct m_allocation*)mm_data_tail(current);
                newAlloc->size = allocTotalSize;
                newAlloc->p = (void*)(newAlloc + sizeof(struct m_allocation));
                newAlloc->allocated = TRUE;
                newAlloc->type = MEM_ALLOC;
                newAlloc->flags = f;
                
                if((f | MEM_CHECKED) == f)
                {
                    mm_set_alloc_canary(newAlloc);
                }
                
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

void kfreef(void* ptr)
{
    if(ptr == NULL)
    {
        kWriteLog("free() called on NULL pointer.");
        
        return;
    }
    
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
            
            if((current->flags | MEM_CHECKED) == current->flags)
            {
                if(mm_verify_alloc_canary(current) == FALSE)
                {
                    kWriteLog_format1d("Overflow detected at address %d", (uint32_t)current->p);
                    
                    Debugger();
                }
            }
            
            mm_link_allocs(current->previous, current->next);
            
            if((current->flags | MEM_ZEROMEM) == current->flags)
            {
                memset(&current, 0, (uint32_t)current->p + current->size);
            }

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

/**
 * Place bytes at the specific offset in a buffer.
 */
void kmemplace(void* dest, uint32_t offset, const char* data, size_t count)
{
    char* cDest = (char*)dest;
    
    for(size_t i = 0; i < count; i++)
    {
        cDest[i + offset] = data[i];
    }
}

/**
 * Get some bytes from the buffer at a specific offset. The returning buffer
 * 'dest' must be provided as an out parameter. This was done because this 
 * function was made for the free() method and was causing an infinite loop
 * with the free() function needing to free the buffer returned by this function
 * the fix was to allocate 'dest' on the stack, skipping the need to call free
 * from the free function.
 */
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

/**
 * Return the space between two allocs.
 */
uint32_t mm_get_space(struct m_allocation* first, struct m_allocation* second)
{
    return mm_data_head(second) - mm_data_tail(first);
}

/**
 * Return the starting address pointed by an allocation structure.
 */
uint32_t mm_data_head(struct m_allocation* target)
{
    return (uint32_t)target;
    //return (uint32_t)target->p;
}

/**
 * Return the end of the data pointed to by an allocation structure.
 */
uint32_t mm_data_tail(struct m_allocation* target)
{
    return (uint32_t)target->p + target->size;
}

/**
 * Return the amount of space from the end of an allocation to the end of heap.
 */
uint32_t mm_space_to_end(struct m_allocation* target)
{
    return (KERNEL_HEAP_START +  KERNEL_HEAP_LENGTH) - mm_data_tail(target);
}

/**
 * Link two allocations together.
 */
void mm_link_allocs(struct m_allocation* first, struct m_allocation* second)
{
    first->next = second;
    second->previous = first;
}

/**
 * Scan the list of allocation and return the first one who is free.
 * It needs to scan the list linearly so it gets slower as more allocations 
 * come into play.
 */
struct m_allocation* mm_find_free_allocation()
{
    return NULL;
}
struct m_allocation* mm_find_free_space(size_t bytes)
{
    struct m_allocation* current = firstAlloc;
    while(current != NULL)
    {
        struct m_allocation* next = current->next;
        
        if(mm_get_space(current, next) >= bytes)
        {
            return (struct m_allocation*)mm_data_tail(current);
        }
        
    }
    
    return NULL;
}

/**
 * Set the last X bytes of an allocation to a specific sequence of bytes.
 * That sequence of bytes is checked when it is freed or called manually. If the 
 * special bytes have been modified then we have detected a buffer overflow.
 */
void mm_set_alloc_canary(struct m_allocation* alloc)
{
    //alloc->size += MM_HEAP_ALLOC_CANARY_SIZE;
    
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

/**
 * Trigger a search of all allocations and returns FALSE as soon as a buffer 
 * overflow is detected.
 */
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
