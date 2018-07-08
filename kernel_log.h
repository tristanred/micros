#ifndef KERNEL_LOG_H
#define KERNEL_LOG_H

#include "serial.h"

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

COMPRT keLogPort;

void kSetupLog(COMPRT loggingPort);

void kWriteLog(const char* str, ...);

#endif
