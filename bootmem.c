#include "bootmem.h"

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

    
    mtbInfo = mbi;
    
    // int len = arg1->mmap_length;
    // void* addr = arg1->mmap_addr;
    //
    // multiboot_memory_map_t* x = (multiboot_memory_map_t*)mbi;
    //
    // multiboot_memory_map_t* y = x + 1;
    // multiboot_memory_map_t* z = x + 2;

}
