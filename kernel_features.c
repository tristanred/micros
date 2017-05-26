#include "kernel_features.h"


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
        char* CommandlineText = (unsigned char*)info->cmdline;
        
        parse_commandline(CommandlineText);
        
    }
}

void parse_commandline(unsigned char* cmdline)
{
    
}

BOOL kfSupportGraphics()
{
    
}

BOOL kfDebugMode()
{
    
}
