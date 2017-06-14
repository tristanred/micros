#ifndef ARRAY_UTILS_H
#define ARRAY_UTILS_H

#include <stddef.h>
#include <stdint.h>

uint32_t array_emplace(uint8_t* destination, uint8_t* source, uint32_t startIndex, uint32_t length);

#endif
