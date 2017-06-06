#include "kernel_features.h"

#include "memory.h"
#include "string.h"
#include "multiboot.h"
#include "kernel.h"
#include "cmd_parser.h"
#include "kernel_log.h"

void init_module_kernel_features(struct kernel_info_block* kinfo)
{
    // INFO : Template module initialization
    // 1. use alloc_kernel_module to place the structure at the correct memory location
    // 2. Link the structure in the kernel info block.
    features = alloc_kernel_module(sizeof(struct kernel_features));
    kinfo->m_kernel_features = features;
    
    features->current_graphic_mode = TEXT;
    features->isDebugBuild = FALSE;
    features->kernel_options_size = 0;
    features->kernel_options = NULL;
}

void kfDetectFeatures(multiboot_info_t* info)
{
    kWriteLog("PRINTING MULTIBOOT OPTIONS\n");
    
    kWriteLog_format1d("Multiboot flags = %d\n", info->flags);

    if(info->flags & 0)
    {
        //MemoryInfo = TRUE;
        features->availableLowerMemory = info->mem_lower;
        features->availableUpperMemory = info->mem_upper;
        
        kWriteLog_format1d("Available lower memory %d\n", info->mem_lower);
        kWriteLog_format1d("Available upper memory %d\n", info->mem_upper);
    }
    if(info->flags & 1)
    {
        //BootDevice = TRUE;
        uint8_t* parts = (uint8_t*)info->boot_device;
        
        kWriteLog_format1d("Boot Device drive %d\n", parts[0]);
        kWriteLog_format1d("Boot Device part1 %d\n", parts[1]);
        kWriteLog_format1d("Boot Device part2 %d\n", parts[2]);
        kWriteLog_format1d("Boot Device part3 %d\n", parts[3]);
        
    }
    if(info->flags & 2)
    {
        //CmdLine = TRUE;
        char* CommandlineText = (char*)info->cmdline;
        
        kWriteLog("CommandLine : ");
        kWriteLog(CommandlineText);
        kWriteLog("");
        
        if(validate_commandline(CommandlineText))
        {
            features->kernel_options = parse_commandline(CommandlineText, &features->kernel_options_size);
            
            activate_options(features->kernel_options, features->kernel_options_size);
        }
        else
        {
            
        }
        
    }
    if(info->flags & 3)
    {
        kWriteLog("Multiboot modules\n");
        
        struct multiboot_mod_list* mods = (struct multiboot_mod_list*)info->mods_addr;
        
        for(multiboot_uint32_t i; i < info->mods_count; i++)
        {
            kWriteLog_format1d("Module %d : ", i);
            kWriteLog_format1d("Module Start = %d", mods[i].mod_start);
            kWriteLog_format1d("Module End = %d", mods[i].mod_end);
            kWriteLog("Module Commandline : ");
            
            char* x = (char*)mods[i].cmdline;
            kWriteLog(x);
        }
    }
    if(info->flags & 6)
    {
        // mem map
    }
    if(info->flags & 7)
    {
        // drives
    }
    if(info->flags & 8)
    {
        // config table
    }
    if(info->flags & 9)
    {
        // boot loader name
    }
    if(info->flags & 10)
    {
        
    }
    if(info->flags & 11)
    {
        
    }
}

/**
 * Format of the commandline arguments
 *
 * out/myos.bin -f dg
 * ^            ^
 * |            -f : build features
 * |                 d = debug ON
 * |                 g = vga graphics ON
 * kernel filename
 * 
 * 
 *
 */

/**
 * Currently only checking for string length under 256 because commandline
 * parser is restricted for arguments under 256 chars.
 */
BOOL validate_commandline(char* cmdline)
{
    size_t length = strlen(cmdline);
    
    return length < 256;
}

void activate_options(char** cmdline, int size)
{
    for(int i = 0; i < size; i++)
    {
        char* argument = cmdline[i];
        
        if(strncmp(argument, "-f", 2) == 0)
        {
            size_t amount;
            char** feature = strspl(argument, " ", &amount);
            
            if(amount >= 2)
            {
                activate_features(feature[1]);
            }
            
            for(size_t k = 0; k < amount; k++)
            {
                free(feature[k]);
            }
            
            free(feature);
        }
    }
}

void activate_features(char* feature_arg)
{
    size_t length = strlen(feature_arg);
    
    for(size_t i = 0; i < length; i++)
    {
        switch(feature_arg[i])
        {
            case 'd':
            {
                features->isDebugBuild = TRUE;
                
                break;
            }
            case 'g':
            {
                features->current_graphic_mode = VGA_GRAPHICS;
                
                break;
            }
        }
    }
    
}

BOOL kfSupportGraphics()
{
    return FALSE;
}

BOOL kfDebugMode()
{
    return TRUE;
}
