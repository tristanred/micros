#ifndef KERNEL_FEATURES_H
#define KERNEL_FEATURES_H

#include <stddef.h>
#include <stdint.h>

#include "multiboot.h"
#include "common.h"

// Private Members
enum graphical_modes {
    TEXT,
    VGA_GRAPHICS,
    NONE
};

typedef struct {
    enum graphical_modes current_graphic_mode;
    
    BOOL isDebugBuild;
    
    size_t kernel_options_size;
    unsigned char** kernel_options;
    
} kernel_features;

void parse_commandline(unsigned char* cmdline);

// Public Members
void kfDetectFeatures(multiboot_info_t* info);
BOOL kfSupportGraphics();
BOOL kfDebugMode();

#endif
