#include "array_utils.h"
#include "memory.h"

uint32_t array_emplace(uint8_t* destination, uint8_t* source, uint32_t startIndex, uint32_t length)
{
    uint32_t bytesWritten = 0;
    
    for(uint32_t i = 0; i < length; i++)
    {
        destination[i + startIndex] = source[i];
        
        bytesWritten++;
    }
    
    return bytesWritten;
}

void array_zero(uint8_t* destination, int length)
{
    array_set(destination, 0, length);
}

void array_set(uint8_t* destination, uint8_t value, int length)
{
    for(int i = 0; i < length; i++)
    {
        destination[i] = value;
    }
}

void array_set_pattern(uint8_t* destination, uint8_t* pattern, int destinationLength, int patternLength)
{
    for(int i = 0; i < destinationLength; i++)
    {
        destination[i] = pattern[i % patternLength];
    }
}

uint8_t* array_concat(uint8_t* first, size_t firstSize, uint8_t* second, size_t secondSize)
{
    uint8_t* res = (uint8_t*)malloc(sizeof(uint8_t) * firstSize + secondSize);
    
    for(size_t i = 0; i < firstSize; i++)
    {
        res[i] = first[i];
    }
    
    for(size_t i = 0; i < secondSize; i++)
    {
        res[i + firstSize] = second[i];
    }
    
    return res;
}