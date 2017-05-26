#ifndef BOOTINFO_H
#define BOOTINFO_H

#include <stddef.h>
#include <stdint.h>

#include "multiboot.h"
#include "common.h"

// Flags
uint32_t flags;

BOOL MemoryInfo;
BOOL BootDevice;
BOOL CmdLine;
BOOL BootModules;
BOOL SymsTables;
BOOL MemoryMaps;
BOOL DriveInfo;
BOOL RomConfig;
BOOL BootloaderName;
BOOL ApmTable;
BOOL VideoInfo;

// Memory Info
// Amount of mem available above 0 and below 640
uint32_t availableLowerMemory;
// Is maximally the address of the first upper memory hole minus 1 megabyte
uint32_t availableUpperMemory;

// Boot Device info

// Command Line
unsigned char* CommandlineText;

void ReadMultibootFlags(multiboot_info_t* info);

#endif
