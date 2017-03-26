#include "string.h"
#include "memory.h"

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

int sprintf_1d(char* buffer, const char* format, int number)
{
    size_t formatSize = strlen(format);
    char* numberString = strdigits(number);
    size_t numberStringSize = strlen(numberString);
    
    size_t formatCounter = 0;
    size_t bufferCounter = 0;
    
    size_t stopPoint = formatSize - 2 + numberStringSize;
    
    int go = 0;
    while(go == 0)
    {
        if(format[bufferCounter] == '%' && format[bufferCounter + 1] == 'd')
        {
            formatCounter += 2;
            
            size_t numberCounter = 0;
            for(numberCounter = 0; numberCounter < numberStringSize; numberCounter++)
            {
                buffer[bufferCounter++] = numberString[numberCounter];
            }
        }
        else
        {
            buffer[bufferCounter++] = format[formatCounter++];
        }
        
        if(bufferCounter >= stopPoint)
        {
            go = 1;
        }
    }
    
    buffer[stopPoint] = '\0';
    
    return stopPoint;    
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
    
    dest[srcSize] = '\0';
    
    return dest;
}

char* strrev(char* str)
{
    size_t len = strlen(str);
    char* reverse = kmKernelAlloc(sizeof(char) * len + 1);
    
    for(size_t i = 0; i < len; i++)
    {
        reverse[i] = str[(len - 1) - i];
    }
    
    reverse[len] = '\0';
    
    return reverse;
}

char* strdigits(uint32_t number)
{
    char result[256];
    int digitCounter = 0;
    int divider = number;
    
    while(divider != 0)
    {
        int remainder = divider % 10;
        
        char digitCode = remainder + 48;
        
        divider = divider / 10;
        
        result[digitCounter++] = digitCode;
    }
    
    char* reversedResult = strrev(result);
    
    char* outResult = kmKernelAlloc(sizeof(char) * digitCounter + 1);
    strcpy(outResult, reversedResult);
    
    return outResult;
}
