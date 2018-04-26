#include "bootmem.h"

#include "kernel_log.h"
#include "types.h"
#include "flagutils.h"

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

void mbt_pretty_print_info(multiboot_info_t* mbi)
{
    kWriteLog("***** MULTIBOOT DETAILS *****");
    
    kWriteLog_format1d_stacksafe("Flags : %d", mbi->flags);
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_MEMORY))
        kWriteLog("  BIOS Memory Info = Present");
    else
        kWriteLog("  BIOS Memory Info = Absent");
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_BOOTDEV))
        kWriteLog("  Boot Dev info = Present");
    else
        kWriteLog("  Boot Dev info = Absent");
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_CMDLINE))
        kWriteLog("  Commandline = Present");
    else
        kWriteLog("  Commandline = Absent");
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_MODS))
        kWriteLog("  Modules = Present");
    else
        kWriteLog("  Modules = Absent");
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_AOUT_SYMS))
        kWriteLog("  Symbol Table = Present");
    else
        kWriteLog("  Symbol Table = Absent");
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_ELF_SHDR))
        kWriteLog("  ELF Section = Present");
    else
        kWriteLog("  ELF Section = Absent");
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_MEM_MAP))
        kWriteLog("  Memory Map = Present");
    else
        kWriteLog("  Memory Map = Absent");
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_DRIVE_INFO))
        kWriteLog("  Drive Info = Present");
    else
        kWriteLog("  Drive Info = Absent");
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_CONFIG_TABLE))
        kWriteLog("  Config Table = Present");
    else
        kWriteLog("  Config Table = Absent");
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_BOOT_LOADER_NAME))
        kWriteLog("  Boot Loader name = Present");
    else
        kWriteLog("  Boot Loader name = Absent");
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_APM_TABLE))
        kWriteLog("  APM Table = Present");
    else
        kWriteLog("  APM Table = Absent");
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_VIDEO_INFO))
        kWriteLog("  Video Info = Present");
    else
        kWriteLog("  Video Info= Absent");
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_MEMORY))
    {
        kWriteLog("Memory Map details");
        kWriteLog_format1d_stacksafe("  Mem lower : %d", mbi->mem_lower);
        kWriteLog_format1d_stacksafe("  Mem upper : %d", mbi->mem_upper);
    }
    else
    {
        kWriteLog("Memory Map details not available");
    }
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_BOOTDEV))
    {
        kWriteLog("Boot Device details");
        kWriteLog_format1d_stacksafe("  Boot Dev : %d", mbi->boot_device);
    }
    else
    {
        kWriteLog("Boot Device details not available");
    }
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_CMDLINE))
    {
        kWriteLog("Commandline details");
        kWriteLog("  Boot commandline: ");
        char* cmd = (char*)mbi->cmdline;
        kWriteLog(cmd);
    }
    else
    {
        kWriteLog("Commandline details not available.");
    }
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_MODS))
    {
        kWriteLog("Modules details");
        kWriteLog_format1d_stacksafe("  Mods count : %d", mbi->mods_count);
        kWriteLog_format1d_stacksafe("  Mods addr : %d", mbi->mods_addr);
    
        if(mbi->mods_count > 0)
        {
            kWriteLog("  Modules list : ");
            
            mbt_print_modules_list(mbi);
        }
    }
    else
    {
        kWriteLog("Module details not available.");
    }
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_AOUT_SYMS))
    {
        kWriteLog("AOUT Section");
        kWriteLog("  Not yet implemented");
    }
    else if(FLAG(mbi->flags, MULTIBOOT_INFO_ELF_SHDR))
    {
        kWriteLog("ELF Section");
        kWriteLog_format1d_stacksafe("  Addr : %d", mbi->u.elf_sec.addr);
        kWriteLog_format1d_stacksafe("  Num : %d", mbi->u.elf_sec.num);
        kWriteLog_format1d_stacksafe("  SH idx : %d", mbi->u.elf_sec.shndx);
        kWriteLog_format1d_stacksafe("  Size : %d", mbi->u.elf_sec.size);
    }
    else
    {
        kWriteLog("AOUT and ELF details not available.");
    }
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_MEM_MAP))
    {
        kWriteLog("Memory Map details");
        kWriteLog_format1d_stacksafe("  Mem Map Len : %d", mbi->mmap_length);
        kWriteLog_format1d_stacksafe("  Mem Map Addr : %d", mbi->mmap_addr);
        
        //mbt_print_zones(mbi);
    }
    else
    {
        kWriteLog("No Memory Map details");
    }
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_DRIVE_INFO))
    {
        kWriteLog("Drive Info details");
        kWriteLog_format1d_stacksafe("  Drive Len : %d", mbi->drives_length);
        kWriteLog_format1d_stacksafe("  Drive Addr : %d", mbi->drives_addr);

        mbt_print_drives(mbi);
    }
    else
    {
        kWriteLog("Drive info details not available.");
    }
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_CONFIG_TABLE))
    {
        kWriteLog("Config table detail");
        kWriteLog_format1d_stacksafe("  Config Table addr : %d", mbi->config_table);
    }
    else
    {
        kWriteLog("Config table details not available");
    }
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_BOOT_LOADER_NAME))
    {
        kWriteLog("Boot loader details");
        kWriteLog("  Boot loader name:");
        kWriteLog((char*)mbi->boot_loader_name);
    }
    else
    {
        kWriteLog("Boot loader details not available");
    }
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_APM_TABLE))
    {
        kWriteLog("APM Table details");
        kWriteLog_format1d_stacksafe("  APM Table : %d", mbi->apm_table);

        mbt_print_apm_table(mbi);
    }
    else
    {
        kWriteLog("APM Table details not avilable");
    }
    
    if(FLAG(mbi->flags, MULTIBOOT_INFO_VIDEO_INFO))
    {
        kWriteLog("VBE details");
        kWriteLog_format1d_stacksafe("  VBE Control info : %d", mbi->vbe_control_info);
        kWriteLog_format1d_stacksafe("  VBE Mode info : %d", mbi->vbe_mode_info);
        kWriteLog_format1d_stacksafe("  VBE Mode : %d", mbi->vbe_mode);
        kWriteLog_format1d_stacksafe("  VBE interface segment : %d", mbi->vbe_interface_seg);
        kWriteLog_format1d_stacksafe("  VBE Control offset : %d", mbi->vbe_interface_off);
        kWriteLog_format1d_stacksafe("  VBE Control length : %d", mbi->vbe_interface_len);

        mbt_print_vbe_info(mbi);
    }
    else
    {
        kWriteLog("VBE details not available");
    }
    

    kWriteLog("***** MULTIBOOT DETAILS *****");
}

void mbt_print_modules_list(multiboot_info_t* mbi)
{
    multiboot_module_t* start = (multiboot_module_t*)mbi->mods_addr;
    
    for(multiboot_uint32_t i = 0; i < mbi->mods_count; i++)
    {
        multiboot_module_t* module = start + i;
        
        kWriteLog_format1d_stacksafe("Module Start : %d", module->mod_start);
        kWriteLog_format1d_stacksafe("Module End : %d", module->mod_end);
        kWriteLog("Module commandline :");
        kWriteLog((char*)module->cmdline);
        kWriteLog_format1d_stacksafe("Padding : %d", module->pad);
    }
}

void mbt_print_drives(multiboot_info_t* mbi)
{
    
}

void mbt_print_apm_table(multiboot_info_t* mbi)
{
    
}


void mbt_print_vbe_info(multiboot_info_t* mbi)
{
    
}
