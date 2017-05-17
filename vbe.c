#include "vbe.h"

void ReadMultibootFlags(multiboot_info_t* info)
{
    if(info->flags & 0x1)
    {
        MemoryInfo = TRUE;
        availableLowerMemory = info->mem_lower;
        availableUpperMemory = info->mem_upper;
    }
    if(info->flags & 0x2)
    {
        BootDevice = TRUE;
    }
    if(info->flags & 0x4)
    {
        CmdLine = TRUE;
        CommandlineText = (unsigned char*)info->cmdline;
    }
    if(info->flags & 0x8)
    {
        BootModules = TRUE;
    }
    if(info->flags & 0x16 || info->flags & 0x32)
    {
        SymsTables = TRUE;
    }
    if(info->flags & 0x64)
    {
        MemoryMaps = TRUE;
    }
    if(info->flags & 0x128)
    {
        DriveInfo = TRUE;
    }
    if(info->flags & 0x256)
    {
        RomConfig = TRUE;
    }
    if(info->flags & 0x512)
    {
        BootloaderName = TRUE;
    }
    if(info->flags & 0x1024)
    {
        ApmTable = TRUE;
    }
    if(info->flags & 0x2048)
    {
        VideoInfo = TRUE;
    }
}