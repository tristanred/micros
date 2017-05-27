#include "kernel_features.h"

#include "multiboot.h"
#include "kernel.h"

void init_module_kernel_features(struct kernel_info_block* kinfo)
{
    features = &kinfo->m_kernel_features;
    
    features->current_graphic_mode = 0;
    features->isDebugBuild = FALSE;
    features->kernel_options_size = 0;
    features->kernel_options = NULL;
}

void kfDetectFeatures(multiboot_info_t* info)
{
    if(info->flags & 1)
    {
        //MemoryInfo = TRUE;
        //availableLowerMemory = info->mem_lower;
        //availableUpperMemory = info->mem_upper;
    }
    if(info->flags & 2)
    {
        //BootDevice = TRUE;
    }
    if(info->flags & 4)
    {
        //CmdLine = TRUE;
        unsigned char* CommandlineText = (unsigned char*)info->cmdline;
        
        parse_commandline(CommandlineText);
        
    }
}

void parse_commandline(unsigned char* cmdline)
{
    (void)cmdline;
}

BOOL kfSupportGraphics()
{
    return FALSE;
}

BOOL kfDebugMode()
{
    return TRUE;
}
