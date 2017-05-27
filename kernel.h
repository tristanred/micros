#ifndef KERNEL_H
#define KERNEL_H

#include "kernel_features.h"

/**
 * The Kernel Info Block is the root structure of the kernel. This structure
 * contains all the other kernel structures and running kernel information.
 * This structure IS the Kernel and should be constanyly updated to reflect the
 * current state.
 */
struct kernel_info_block {
    struct kernel_features m_kernel_features;
};
struct kernel_info_block kernel_info;

#endif
