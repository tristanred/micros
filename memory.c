#include "memory.h"

#include "kernel_log.h"
#include "task.h"
#include "bootmem.h"

/**
 * The heap manager manages heaps of memory.
 *
 * Blocks of memory are reserved by calling 'kmalloc'. That will create an
 * instance of m_allocation and place it on the heap. Following that data will
 * be the actual bytes of the allocation.
 *
 * All allocations are kept in a linked list. We keep the first and last nodes
 * in the kernel module structure (todo). The linked list is kept ordered in
 * order of ascending addresses.  When a new allocation is placed between
 * blocks of memory, the allocation will be linked with the proper next and
 * previous.
 *
*/

/*
 * Module init function.
 *
 * Must be called before any memory calls are done.
 *
 */
void init_memory_manager(struct kernel_info_block* kinfo, multiboot_info_t* mbi)
{
    mm_module = (struct memory_manager_module*)alloc_kernel_module(sizeof(struct memory_manager_module));
    kinfo->m_memory_manager = mm_module;

    // uint32_t startingHeapPage = KERNEL_HEAP_START; // TODO
    // uint32_t heapPageCount = KERNEL_HEAP_LENGTH / PAGE_SIZE; // TODO

    // for(uint32_t i = 0; i < heapPageCount; i++)
    // {
    //     pa_pt_alloc_pageaddr(pa_get_current_pt(), startingHeapPage + (i * PAGE_SIZE));
    // }

    mbt_print(mbi);
    mbt_pretty_print_info(mbi);
    mbt_print_zones(mbi);
    // Find largest memory zone
    uint32_t zone_start = 0;
    uint32_t zone_length = 0;
    mm_zone_find_largest(mbi, &zone_start, &zone_length);

    mm_module->memory_start = zone_start;
    mm_module->memory_length = zone_length;

    ASSERT(zone_start != 0, "Invalid zone start");
    ASSERT(zone_start != 0, "Invalid zone end");

    // Calculate how big our memory zones are. Mostly used for the heap.
    memset(&mm_module->zones, 0, sizeof(struct mm_zonedata));
    mm_calculate_zones(zone_start, zone_length, &mm_module->zones);

    struct m_heap* kheap = (struct m_heap*)mm_module->zones.kheap_start;
    kheap->hflags = FHEAP_NONE;
    kheap->startAddress = mm_module->zones.kheap_start + sizeof(struct m_heap);
    kheap->endAddress = mm_module->zones.kheap_end;
    kheap->allocs_count = 0;
    kheap->firstAlloc = NULL;
    kheap->lastAlloc = NULL;

    mm_module->kernel_heap = kheap;
    
    vector_setup(&mm_module->heaps);
}

/**
 * Allocate memory on the heap.
 */
void* kmalloc(uint32_t size)
{
    return kmallocf(size, MEM_NOFLAGS, HEAP);
}

/**
 * Release memory allocated by malloc.
 */
void kfree(void* ptr)
{
    kfreef(ptr, HEAP);
}

void* vmalloc(uint32_t size)
{
    // TODO : Get proc rather than task
    struct task_t* procHeap = ks_get_current();
    
    if(procHeap == NULL)
    {
        BUG("Current Process is null when calling vmalloc");
    }
    
    return kmallocf(size, MEM_NOFLAGS, procHeap->task_heap);
}

void vmfree(void* p)
{
    // TODO : Get proc rather than task
    struct task_t* procHeap = ks_get_current();
    
    if(procHeap == NULL)
    {
        BUG("Current Process is null when calling vmfree");
    }

    kfreef(p, procHeap->task_heap);
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
 * Same as kmalloc but with upgraded functionalities.
 *
 * The kmalloc function is meant to call this one with the default flag values.
 *
 */
void* kmallocf(uint32_t size, enum mm_alloc_flags f, struct m_heap* target)
{
    uint32_t allocTotalSize = size;
    if((f | MEM_CHECKED) == f)
        allocTotalSize += MM_HEAP_ALLOC_CANARY_SIZE;

    if(target->firstAlloc == NULL)
    {
        struct m_allocation* newAlloc = (struct m_allocation*)target->startAddress;
        newAlloc->size = allocTotalSize;
        newAlloc->p = (void*)(newAlloc + sizeof(struct m_allocation));
        newAlloc->allocated = TRUE;
        newAlloc->type = MEM_ALLOC;
        newAlloc->flags = f;

        if((f | MEM_CHECKED) == f)
        {
            mm_set_alloc_canary(newAlloc);
        }

        target->firstAlloc = newAlloc;
        target->lastAlloc = newAlloc;

        return newAlloc->p;
    }

    struct m_allocation* current = target->firstAlloc;
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

                target->lastAlloc = newAlloc;

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

                target->lastAlloc = newAlloc;

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

/**
 * Release of pointer of memory. Meant to be called from 'kfree' with default
 * flags. This version of the function currently does not take any flags but
 * it might at some point.
 */
void kfreef(void* ptr, struct m_heap* target)
{
    if(ptr == NULL)
    {
        kWriteLog("free() called on NULL pointer.");

        return;
    }

    struct m_allocation* current = target->lastAlloc;
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
                    kWriteLog("Overflow detected at address %d", (uint32_t)current->p);
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

struct m_heap* mm_create_heap(enum heap_flags flags)
{
    // Heap creation steps.
    // Firsts we need to find space in the memory for the heap.
    // Check between each heap to see if theres a fit, if not check between
    // the last heap and the end of the memory zone.
    // This is pretty much the same algorithm as with the memory allocations.
    uint32_t newHeapAddress = mm_find_heap_space(DEFAULT_HEAP_SPACE);

    if(newHeapAddress == 0)
    {
        // out of memory 
        return NULL;
    }
    
    struct m_heap* newHeap = kmallocf(sizeof(struct m_heap), FHEAP_NONE, mm_module->kernel_heap);
    newHeap->startAddress = (uint32_t)newHeap + sizeof(struct m_heap);
    newHeap->endAddress = newHeap->startAddress + DEFAULT_HEAP_SPACE - sizeof(struct m_heap);
    newHeap->hflags = flags;
    newHeap->allocs_count = 0;
    newHeap->firstAlloc = NULL;
    newHeap->lastAlloc = NULL;

    vector_add(&mm_module->heaps, newHeap);
    
    return newHeap;
}

void mm_zone_find_largest(multiboot_info_t* mbi, uint32_t* start, uint32_t* length)
{
    multiboot_uint32_t len = mbi->mmap_length;
    void* addr = (void*)mbi->mmap_addr;

    multiboot_uint32_t zone_length = 0;
    multiboot_uint32_t zone_addr = 0;

    // Find the largest zone of memory, type 1 memory is available memory
    for(multiboot_uint32_t i = 0; i < len / sizeof(multiboot_memory_map_t); i++)
    {
        multiboot_memory_map_t* x = (multiboot_memory_map_t*)addr + i;

        if(x->len > zone_length && x->type == 1)
        {
            zone_length = x->len;
            zone_addr = x->addr;
        }
    }

    *start = zone_addr;
    *length = zone_length;
}

void mm_calculate_zones(uint32_t start, uint32_t length, struct mm_zonedata* data)
{
    (void)start;
    int32_t amountLeft = length;

    // Bios area is always the first 640k, extended to 1MB.
    // Also we don't count the BIOS in the memory left.
    data->bios_start = 0;
    data->bios_end = 1024*1024;

    // Code and data sections. Takes one MB
    uint32_t sections_size = 1024*1024;
    data->sections_start = data->bios_end;
    data->sections_end = data->sections_start + sections_size;
    amountLeft -= sections_size;

    uint32_t pfm_size = 1024*1024;
    data->pfm_start = data->sections_end;
    data->pfm_end = data->pfm_start + pfm_size;
    amountLeft -= pfm_size;

    uint32_t kernel_pagetable_size = 1024*1024*4;
    data->kpt_start = data->pfm_end;
    data->kpt_end = data->kpt_start + kernel_pagetable_size;
    amountLeft -= kernel_pagetable_size;

    uint32_t kernel_modules_size = 1024*1024;
    data->modules_start = data->kpt_end;
    data->modules_end = data->modules_start + kernel_modules_size;
    amountLeft -= kernel_modules_size;

    /** Strategy to find an appropriate kernel heap size. This step is a bit
     * more involved because the more RAM we have, the heavier the system load
     * can be so more kernel heap will be needed to track all those things.
     *
     * This is 100% going to change but for now here is the strategy :
     * 30% of remaining RAM and a minimum of 10 MB.
     * Both of those numbers are completely arbitrary. The only accurate number
     * in this comment is the percent chance that this algorithm will change.
     */
    uint32_t requested_heap_size = amountLeft * 0.30;
    if(requested_heap_size < 1024*1024*10)
    {
        requested_heap_size = 1024*1024*10;
    }

    data->kheap_start = data->modules_end;
    data->kheap_end = data->kheap_start + requested_heap_size;
    amountLeft -= requested_heap_size;

    if(amountLeft < 0)
    {
        // No abs() function yet. Also sprintf function can't handle negatives
        uint32_t abs_amountleft = amountLeft * -1;
        kWriteLog("Unable to allocate enough memory for the kernel.\n");
        kWriteLog("Missing memory: %d bytes.", abs_amountleft);

        PanicQuit("MEMORY INIT ERROR.");
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
    return (HEAP->startAddress + HEAP_LENGTH) - mm_data_tail(target);
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

/**
 * Find a section of the heap big enough to contain the amount of bytes.
 *
 * Will start with the first allocations to try and find space in between
 * existing allocations first.
 *
 * TODO : Looks like the function won't check if space exist between the last
 * allocation and the end of memory. FIXME
 */
struct m_allocation* mm_find_free_space(size_t bytes)
{
    struct m_allocation* current = HEAP->firstAlloc;
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
    kmemplace(alloc->p, alloc->size - MM_HEAP_ALLOC_CANARY_SIZE, MM_HEAP_ALLOC_CANARY_VALUE, MM_HEAP_ALLOC_CANARY_SIZE);
}

/**
 * Returns true if the canary was untouched, so no buffer overflow (detected).
 * A buffer overflow can still happen if it skips the X canary bytes before
 * writing bad data.
 *
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
 * overflow is detected. This indicates that some data somewhere have an
 * overflow.
 */
BOOL mm_verify_all_allocs_canary()
{
    struct m_allocation* current = HEAP->firstAlloc;
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

uint32_t mm_find_heap_space(size_t heapSize)
{
    // If there are no heaps yet, give the space right after the kernel heap
    if(mm_module->heaps.count == 0)
    {
        return mm_module->kernel_heap->endAddress;
    }
    
    struct m_heap* current = vector_get_at(&mm_module->heaps, 0);
    for(size_t i = 1; i < mm_module->heaps.count; i++)
    {
        struct m_heap* next = vector_get_at(&mm_module->heaps, i);
        
        if(next == NULL)
        {
            return current->endAddress;
        }
        else
        {
            if(mm_heaps_spacing(current, next) >= heapSize)
            {
                return current->endAddress;
            }
            
            current = next;
        }
    }
    
    PanicQuit("Cannot allocate heap.");
    return 0;
}

uint32_t mm_heaps_spacing(struct m_heap* first, struct m_heap* second)
{
    return second->startAddress - first->endAddress;
}

uint32_t mm_heaps_space_to_end(struct m_heap* lastHeap)
{
    return (mm_module->memory_start + mm_module->memory_length) - lastHeap->endAddress;
}

