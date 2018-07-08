#include "string.h"

#include "memory.h"
#include "vector.h"

// Return the amount of characters of the string, not counting the 
// null terminator.
size_t strlen(const char* str)
{
    size_t len = 0;
    while(str[len])
    {
        len++;
    }
        
    return len;
}

size_t strlen_s(const char* str, size_t strsz)
{
    if(str == 0)
        return 0;
    
    size_t len = 0;
    while(str[len] && len < strsz)
    {
        len++;
    }
        
    return len;
}

int sprintf( char *buffer, const char *format, ...)
{
    // Initialize the vadiaric argument structure
    va_list formatlist;
    va_start(formatlist, format);
    
    int resultLength = vsprintf(buffer, format, formatlist);
    
    va_end(formatlist);
    
    return resultLength;
}

int vsprintf( char *buffer, const char *format, va_list vlist )
{
    size_t sourceLength = strlen(format); // Maximum length we are going to read from 'format'
    size_t sourceIndex = 0; // Position in the input buffer.
    size_t bufferIndex = 0; // Position in the result buffer.
    
    int go = 0;
    while(go == 0)
    {
        // Detect if we're reading in a format specifier
        if(format[sourceIndex] == '%' && format[sourceIndex + 1] == 'd')
        {
            // We take in the format and skip after it in the reading.
            sourceIndex += 2;
            
            // Get the number we're going to place in 'buffer'
            uint32_t numberArgument = va_arg(vlist, uint32_t);
            char numberDigits[256];
            strdigits(numberArgument, numberDigits);
            size_t digitsLen = strlen(numberDigits);
            
            // Place the number string to the target buffer.
            size_t numberCounter = 0;
            for(numberCounter = 0; numberCounter < digitsLen; numberCounter++)
            {
                buffer[bufferIndex++] = numberDigits[numberCounter];
            }
        }
        else
        {
            // If not reading a format specifier, continue placing source
            // characters into buffer.
            buffer[bufferIndex++] = format[sourceIndex++];
        }
        
        // When we're done reading from 'format' get out !
        if(sourceIndex >= sourceLength)
        {
            go = 1;
        }
    }
    
    buffer[bufferIndex] = '\0'; // Needed or not ?
    
    return sourceLength;
}

char* strcpy(char* dest, const char* src)
{
    size_t srcSize = strlen(src);
    
    for(size_t i = 0; i < srcSize; i++)
    {
        dest[i] = src[i];
    }
    
    dest[srcSize] = '\0';
    
    return dest;
}

char* strncpy(char* dest, const char* src, size_t count)
{
    size_t srcSize = strlen(src);
    
    size_t minSize = 0;
    
    if(count < srcSize)
    {
        minSize = count;
    }
    else
    {
        minSize = srcSize;
    }
    
    for(size_t i = 0; i < minSize; i++)
    {
        dest[i] = src[i];
    }
    
    dest[minSize] = '\0';
    
    return dest;
}

int strcmp( const char *lhs, const char *rhs )
{
    size_t i = 0;
    while(lhs[i])
    {
        if(lhs[i] != rhs[i])
        {
            if(lhs[i] > rhs[i])
            {
                return 1;
            }
            else
            {
                return -1;
            }
        }
        
        i++;
    }
    
    return 0;
}

int strncmp( const char *lhs, const char *rhs, size_t count )
{
    size_t i = 0;
    while(i < count)
    {
        if(lhs[i] != rhs[i])
        {
            if(lhs[i] > rhs[i])
            {
                return 1;
            }
            else
            {
                return -1;
            }
        }
        
        i++;
    }
    
    return 0;
}

void *memset( void *dest, int ch, size_t count )
{
    // Std lib says ch is converted to a uchar
    unsigned char* destc = (unsigned char*)dest;
    unsigned char value = (unsigned char)ch;
    
    for(size_t i = 0; i < count; i++)
    {
        destc[i] = value;
    }
    
    return dest;
}

BOOL mcmp(uint8_t* lhs, uint8_t* rhs, size_t count)
{
    size_t i = 0;
    while(i < count)
    {
        if(lhs[i] != rhs[i])
        {
            if(lhs[i] > rhs[i])
            {
                return 1;
            }
            else
            {
                return -1;
            }
        }
        
        i++;
    }
    
    return 0;
}

char** strspl(char* buffer, char* separator, size_t* amount)
{
    size_t buflen = strlen(buffer);
    struct vector* elements = vector_create();
    
    size_t index = 0;
    
    size_t accumulatorIndex = 0;
    char* accumulator = (char*)malloc(sizeof(char) * 256);
    
    while(index < buflen)
    {
        if(strncmp(&buffer[index], separator, 1) == 0)
        {
            // Finish the current element
            accumulator[accumulatorIndex] = '\0';
            vector_add(elements, accumulator);
            accumulator = (char*)malloc(sizeof(char) * 256);
            accumulatorIndex = 0;
        }
        else
        {
            accumulator[accumulatorIndex] = buffer[index];
            accumulatorIndex++;
        }
        
        index++;
    }
    
    if(accumulatorIndex > 0)
    {
        accumulator[accumulatorIndex] = '\0';
        vector_add(elements, accumulator);
    }
    
    *amount = elements->count;
    
    char** result = (char**)malloc(elements->count * sizeof(char*));
    
    for(size_t i = 0; i < elements->count; i++)
    {
        result[i] = (char*)vector_get_at(elements, i);
    }
    
    free(elements);
    
    return result;
}

void splfree(char** splitParts, size_t parts)
{
    for(size_t i = 0; i < parts; i++)
    {
        if(splitParts[i] != NULL)
        {
            free(splitParts[i]);
        }
    }
    
    free(splitParts);
}

void strrev(char* str, char* out)
{
    size_t len = strlen(str);
    
    for(size_t i = 0; i < len; i++)
    {
        out[i] = str[(len - 1) - i];
    }
    
    out[len] = '\0';
}

void strdigits(uint64_t number, char* buf)
{
    char result[256];
    uint64_t digitCounter = 0;
    uint64_t divider = number;
    
    if(divider == 0)
    {
        result[0] = '0';
        digitCounter++;
    }
    
    while(divider != 0)
    {
        uint64_t remainder = divider % 10;
        
        char digitCode = remainder + 48;
        
        divider = divider / 10;
        
        result[digitCounter++] = digitCode;
    }
    
    result[digitCounter] = '\0';
    
    strrev(result, buf);
}

uint64_t s_to_d(char* number)
{
    size_t numberLen = strlen(number);
    
    uint64_t result = 0;
    
    size_t magnitude = 1;
    for(int i = numberLen - 1; i >= 0; i--)
    {
        result += (number[i] - 48) * magnitude;
        
        magnitude *= 10;
    }
    
    return result;
}
