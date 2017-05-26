#include "bootinfo.h"

void ReadMultibootFlags(multiboot_info_t* info)
{
    if(info->flags & 1)
    {
        MemoryInfo = TRUE;
        availableLowerMemory = info->mem_lower;
        availableUpperMemory = info->mem_upper;
    }
    if(info->flags & 2)
    {
        BootDevice = TRUE;
    }
    if(info->flags & 4)
    {
        CmdLine = TRUE;
        CommandlineText = (unsigned char*)info->cmdline;
    }
    if(info->flags & 8)
    {
        BootModules = TRUE;
    }
    if(info->flags & 16 || info->flags & 32)
    {
        SymsTables = TRUE;
    }
    if(info->flags & 64)
    {
        MemoryMaps = TRUE;
    }
    if(info->flags & 128)
    {
        DriveInfo = TRUE;
    }
    if(info->flags & 256)
    {
        RomConfig = TRUE;
    }
    if(info->flags & 512)
    {
        BootloaderName = TRUE;
    }
    if(info->flags & 1024)
    {
        ApmTable = TRUE;
    }
    if(info->flags & 2048)
    {
        VideoInfo = TRUE;
    }
}