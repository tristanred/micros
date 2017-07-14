#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>

#include "types.h"

struct kernel_features;
struct memory_manager;
struct ata_driver_info;

/**
 * The Kernel Info Block is the root structure of the kernel. This structure
 * contains all the other kernel structures and running kernel information.
 * This structure IS the Kernel and should be constanyly updated to reflect the
 * current state.
 */
struct kernel_info_block {
    // Kernel modules allocation tracker
    uint32_t modules_start_address;
    uint32_t modules_end_address;
    size_t modules_current_offset;
    
    // Kernel module structs
    struct kernel_features* m_kernel_features;
    struct memory_manager* m_memory_manager;
    struct ata_driver_info* m_ata_driver; // ATA Driver is temporarily a kernel module
};
struct kernel_info_block* kernel_info;

void setup_kernel_block();

void* alloc_kernel_module(size_t size);

BOOL has_free_modules_space();

#endif
