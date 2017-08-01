#ifndef ARRAY_UTILS_H
#define ARRAY_UTILS_H

#include "micros.h"

#include <stddef.h>
#include <stdint.h>

uint32_t array_emplace(uint8_t* destination, uint8_t* source, uint32_t startIndex, uint32_t length);

void array_zero(uint8_t* destination, int length);

void array_set(uint8_t* destination, uint8_t value, int length);

void array_set_pattern(uint8_t* destination, uint8_t* pattern, int destinationLength, int patternLength);

size_t array_verify_length(uint8_t* source);

uint8_t* array_concat(uint8_t* first, size_t firstSize, uint8_t* second, size_t secondSize);

#endif
