#include "kernel_features.h"

#include "string.h"
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
        features->availableLowerMemory = info->mem_lower;
        features->availableUpperMemory = info->mem_upper;
    }
    if(info->flags & 2)
    {
        //BootDevice = TRUE;
    }
    if(info->flags & 4)
    {
        //CmdLine = TRUE;
        unsigned char* CommandlineText = (unsigned char*)info->cmdline;
        
        features->kernel_options = parse_commandline(CommandlineText, features->kernel_options_size);
        
        activate_options(features->kernel_options, features->kernel_options_size);
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

unsigned char** parse_commandline(unsigned char* cmdline, size_t argsSize)
{
    (void)cmdline;
    (void)argsSize;
    
    return NULL;
}

BOOL validate_commandline(unsigned char* cmdline)
{
    (void)cmdline;
    
    return TRUE;
}

void activate_options(unsigned char** cmdline, size_t size)
{
    (void)cmdline;
    (void)size;
}

BOOL kfSupportGraphics()
{
    return FALSE;
}

BOOL kfDebugMode()
{
    return TRUE;
}
