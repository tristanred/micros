#include "bootmem.h"

#include "kernel_log.h"


void btmConfigureMemoryRanges(multiboot_info_t* mbi)
{
    /*
     * The kernel_main method currently receives the Multiboot info structure
     * from the boot.s code, contained in register EBX with the help of
     * the boot loader. The other arguments are to test the presence of
     * other parameters pushed into the method, helpful for debugging.
     */
    
    /*
     * Read the available memory ranges from the multiboot structure.
     * The variable addr contains the starting address of the structures.
     * Right now for example we are checking the first 3 ranges by incrementing
     * the address pointer to go to the next element.
     */
    kWriteLog("***** Multiboot Info listing *****");
    
    mtbInfo = mbi;
    
    kWriteLog("*** Memory Ranges ***");
    multiboot_uint32_t len = mbi->mmap_length;
    void* addr = (void*)mbi->mmap_addr;
    
    kWriteLog_format1d("Multiboot Info length %d.", len);
    kWriteLog_format1d("Multiboot address %d.", (uint32_t)addr);
    
    for(multiboot_uint32_t i = 0; i < len; i++)
    {
        kWriteLog_format1d("Memory range %d memory map.", i);
    
        multiboot_memory_map_t* x = (multiboot_memory_map_t*)addr + i;
        
        kWriteLog_format1d("Size=%d", x->size);
        kWriteLog_format1d("Address=%d", x->addr);
        kWriteLog_format1d("Length=%d", x->len);
        kWriteLog_format1d("Type=%d", x->type);
        
        if(x->size == 0)
        {
            kWriteLog("Found entry with size 0. Presume end of memory map.");
            
            return;
        }
    }
}
