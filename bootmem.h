#ifndef BOOTMEM_H
#define BOOTMEM_H

#include <stddef.h>
#include <stdint.h>

#include "multiboot.h"

multiboot_info_t* mtbInfo;

void btmConfigureMemoryRanges(multiboot_info_t* mbi);

#endif
