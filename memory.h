#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

uint32_t kmNextAvailableMemoryAddress;

void kmInitManager();

void* kmKernelAlloc(size_t size);

void kmKernelFree(void* ptr);

// TODO : Size detection of memory pointed is not yet active, use strcpy if need
// or create a copy method with amount of mem copied.
void kmKernelCopy(void* ptrFrom, void* ptrTo);

void kmKernelZero(void* ptrFrom);

#endif
