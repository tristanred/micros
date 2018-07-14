#ifndef BOOTLOG_H
#define BOOTLOG_H

#include "common.h"
#include "framebuffer.h"

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

/**
 * Called to report progress on the boot phase of the computer.
 * This will print to log and show lines on the framebuffer.
 */
void kBootProgress(const char* str);

#endif