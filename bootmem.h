#ifndef BOOTMEM_H
#define BOOTMEM_H

#include <stddef.h>
#include <stdint.h>

#include "multiboot.h"

void mbt_print(multiboot_info_t* mbi);

void mbt_print_zones(multiboot_info_t* mbi);

#endif
