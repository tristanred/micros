#ifndef ARRAY_UTILS_H
#define ARRAY_UTILS_H

#include "micros.h"

#include <stddef.h>
#include <stdint.h>

/**
 * Array copy method. Copies 'length' bytes from the source array over to
 * destination starting from 'startIndex'. 
 * 
 * The source array start reading at index 0 and it copies bytes to 
 * destination at offset startIndex.
 */
uint32_t array_emplace(uint8_t* destination, uint8_t* source, uint32_t startIndex, uint32_t length);

/**
 * Zero the memory at address 'destination'.
 */
void array_zero(uint8_t* destination, int length);

/**
 * Set the values of the memory pointed by 'destination' to a value.
 */
void array_set(uint8_t* destination, uint8_t value, int length);

/**
 * Set the values of the memory pointed by 'destination' to a pattern of values.
 */
void array_set_pattern(uint8_t* destination, uint8_t* pattern, int destinationLength, int patternLength);

/**
 * Scan the memory pointed by 'source' and count how many elements before a 
 * NULL byte (0) is found.
 */
size_t array_verify_length(uint8_t* source);

/**
 * Return a new array made of the first and the second array put together.
 */
uint8_t* array_concat(uint8_t* first, size_t firstSize, uint8_t* second, size_t secondSize);

#endif
