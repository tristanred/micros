#include "string.h"

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
