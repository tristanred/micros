#ifndef KERNEL_FEATURES_H
#define KERNEL_FEATURES_H

#include <stddef.h>
#include <stdint.h>

struct kernel_info_block;

#include "common.h"
#include "multiboot.h"


// Private Members
enum graphical_modes {
    TEXT,
    VGA_GRAPHICS,
    NONE
};

struct kernel_features {
    // Memory
    uint32_t availableLowerMemory;
    uint32_t availableUpperMemory;
    
    // Build options
    size_t kernel_options_size;
    unsigned char** kernel_options;
    
    enum graphical_modes current_graphic_mode;
    
    BOOL isDebugBuild;
};

struct kernel_features* features;

void init_module_kernel_features(struct kernel_info_block* kinfo);

unsigned char** parse_commandline(unsigned char* cmdline, size_t argsSize);

BOOL validate_commandline(unsigned char* cmdline);

void activate_options(unsigned char** cmdline, size_t size);

// Public Members
void kfDetectFeatures(multiboot_info_t* info);
BOOL kfSupportGraphics();
BOOL kfDebugMode();

#endif