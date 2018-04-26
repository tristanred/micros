#ifndef BOOTMEM_H
#define BOOTMEM_H

#include <stddef.h>
#include <stdint.h>

#include "multiboot.h"

void mbt_print(multiboot_info_t* mbi);

void mbt_print_zones(multiboot_info_t* mbi);

void mbt_pretty_print_info(multiboot_info_t* mbi);

// Private methods
void mbt_print_modules_list(multiboot_info_t* mbi);

void mbt_print_drives(multiboot_info_t* mbi);

void mbt_print_apm_table(multiboot_info_t* mbi);

void mbt_print_vbe_info(multiboot_info_t* mbi);

#endif
