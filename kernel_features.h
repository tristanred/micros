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
    int kernel_options_size;
    char** kernel_options;
    
    enum graphical_modes current_graphic_mode;
    
    BOOL isDebugBuild;
};

struct kernel_features* features;

void init_module_kernel_features(struct kernel_info_block* kinfo);

BOOL validate_commandline(char* cmdline);

void activate_options(char** cmdline, int size);

void activate_features(char* feature_arg);

// Public Members
void kfDetectFeatures(multiboot_info_t* info);
BOOL kfSupportGraphics();
BOOL kfDebugMode();

#endif
