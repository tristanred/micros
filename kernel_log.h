#ifndef KERNEL_LOG_H
#define KERNEL_LOG_H

#include "serial.h"

COMPRT keLogPort;

void kSetupLog(COMPRT loggingPort);

void kWriteLog(const char* str);

#endif
