#include "bootmem.h"

#include "kernel_log.h"

void mbt_print(multiboot_info_t* mbi)
{
    kWriteLog("***** MULTIBOOT INFO *****");
    
    kWriteLog_format1d_stacksafe("Flags : %d", mbi->flags);
    
    kWriteLog_format1d_stacksafe("Mem lower : %d", mbi->mem_lower);
    kWriteLog_format1d_stacksafe("Mem upper : %d", mbi->mem_upper);
    
    kWriteLog_format1d_stacksafe("Boot Dev : %d", mbi->boot_device);
    kWriteLog_format1d_stacksafe("Cmdline : %d", mbi->cmdline);
    
    kWriteLog_format1d_stacksafe("Mods count : %d", mbi->mods_count);
    kWriteLog_format1d_stacksafe("Mods addr : %d", mbi->mods_addr);
    
    kWriteLog("ELF Section");
    kWriteLog_format1d_stacksafe("  Addr : %d", mbi->u.elf_sec.addr);
    kWriteLog_format1d_stacksafe("  Num : %d", mbi->u.elf_sec.num);
    kWriteLog_format1d_stacksafe("  SH idx : %d", mbi->u.elf_sec.shndx);
    kWriteLog_format1d_stacksafe("  Size : %d", mbi->u.elf_sec.size);
    
    kWriteLog_format1d_stacksafe("Mem Map Len : %d", mbi->mmap_length);
    kWriteLog_format1d_stacksafe("Mem Map Addr : %d", mbi->mmap_addr);
    
    kWriteLog_format1d_stacksafe("Drive Len : %d", mbi->drives_length);
    kWriteLog_format1d_stacksafe("Drive Addr : %d", mbi->drives_addr);
    
    kWriteLog_format1d_stacksafe("Config Table : %d", mbi->config_table);
    
    kWriteLog_format1d_stacksafe("Boot Loader : %d", mbi->boot_loader_name);
    
    kWriteLog_format1d_stacksafe("APM Table : %d", mbi->apm_table);
    
    kWriteLog_format1d_stacksafe("VBE Control info : %d", mbi->vbe_control_info);
    kWriteLog_format1d_stacksafe("VBE Mode info : %d", mbi->vbe_mode_info);
    kWriteLog_format1d_stacksafe("VBE Mode : %d", mbi->vbe_mode);
    kWriteLog_format1d_stacksafe("VBE interface segment : %d", mbi->vbe_interface_seg);
    kWriteLog_format1d_stacksafe("VBE Control offset : %d", mbi->vbe_interface_off);
    kWriteLog_format1d_stacksafe("VBE Control length : %d", mbi->vbe_interface_len);
    
    kWriteLog("***** MULTIBOOT INFO *****");
}

void mbt_print_zones(multiboot_info_t* mbi)
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
