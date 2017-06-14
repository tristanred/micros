#include "array_utils.h"

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
