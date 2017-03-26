#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

uint32_t kmNextAvailableMemoryAddress;

void kmInitManager();

void* kmKernelAlloc(size_t size);

void kmKernelFree(void* ptr);

#endif
