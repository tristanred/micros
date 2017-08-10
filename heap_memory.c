#include "heap_memory.h"

/**
 * == Heap Memory Manager ==
 *
 * This memory manager is used to oversee memory allocations inside a large pool
 * of bytes. 
 *
 * It works by allocating blocks of data, the start of the block is the pointer
 * returned by the 'malloc' function and the size of the block is the size of 
 * the memory requested. Each allocation is layed out one after the other.
 * A block can be deleted or reallocated, the block is then freed possibly
 * creating a space between two allocations. New allocations that fits in those
 * memory holes will be placed there.
 
 * All allocations are kept in a 'm_allocation' struct. The struct are nodes in
 * linked lists of allocations. They contain both a pointer to the next and 
 * previous allocation.
 * 
 * The structs are kept in special blocks that are themselves represented with 
 * an instance of 'm_allocation'. This creates a bit of a problem : we need an
 * instance of the struct to represent the block that contains the data of its 
 * own struct. The answer lies in the layout of the allocation blocks. 
 
 * Allocation blocks are arrays of m_allocation structs side by side. The first
 * struct in the array is the struct representing the allocation of the alloc 
 * block. All the following elements in the array represent other normal 
 * allocations made by the system. Each alloc struct used to represent an alloc 
 * block have the property 'next_alloc_block' set to the instance representing 
 * the next list of allocation instances. This gives us the advantage of not 
 * needing to pre-allocate one huge table of the max amount of memory 
 * allocations. We can create a block of allocation pointers for each new 100's
 * of allocations.
 
 * Currently, the system of allocation blocks is kinda wonky and not really 
 * implemented yet. I'll see if I keep the system as it is or I find something 
 * better. 
 * 
 */

void init_memory_manager()
{
    kernel_heap_start = KHEAP_START;
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
        void* res = mm_create_alloc(freeSpace, size);
        
        return res;
    }
    
    return NULL;
}

void kfree(void* ptr)
{
    if(ptr == NULL)
        return;
    
    // Going backward in the list is probably faster because most allocations 
    // are probably short lived.
    struct m_allocation* current = last_alloc;
    
    while(current != NULL)
    {
        if(current->p == ptr && current->type != MEM_ALLOCS_BLOCK)
        {
            current->previous = current->next;
            
            mm_shift_allocs_in(alloc_list_tail, current);
            
            alloc_count--;
            return;
        }
        
        current = current->previous;
    }
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
    first_block->previous = NULL;
    first_block->next = NULL;
    first_block->next_alloc_block = NULL;
    
    first_alloc = first_block;
    last_alloc = first_block;
    
    alloc_count = 1;
    
    alloc_list_tail = first_block;
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
    uint32_t next_alloc_addr = (uint32_t)((struct m_allocation*)alloc_list_tail->p + alloc_count);
    
    alloc_count++;
    
    struct m_allocation* new_alloc = (struct m_allocation*)next_alloc_addr;
    
    new_alloc->size = size;
    new_alloc->p = (void*)startAddress;
    new_alloc->allocated = TRUE;
    new_alloc->type = MEM_ALLOC;
    new_alloc->flags = 0;
    new_alloc->previous = last_alloc;
    new_alloc->next = NULL;
    new_alloc->next_alloc_block = NULL;
    
    last_alloc->next = new_alloc;
    
    last_alloc = new_alloc;
    
    return new_alloc->p;
}

/**
 * 
 */
void mm_shift_allocs_in(struct m_allocation* alloc_list, struct m_allocation* shifted)
{
    
}