#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

size_t strlen(const char* str);

size_t strlen_s(const char* str, size_t strsz);

char* GetNumberDigits(uint32_t number);

#endif
