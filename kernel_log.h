#ifndef KERNEL_LOG_H
#define KERNEL_LOG_H

#include "serial.h"

#include <stddef.h>
#include <stdint.h>

COMPRT keLogPort;

void kSetupLog(COMPRT loggingPort);

void kWriteLog(const char* str);

void kWriteLog_format1d(const char* str, uint64_t number);

void kWriteLog_format1d_stacksafe(const char* str, uint64_t number);


#endif
