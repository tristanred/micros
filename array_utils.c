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
