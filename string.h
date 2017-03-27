#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

// C LIBRARY FUNCTIONS
size_t strlen(const char* str);
size_t strlen_s(const char* str, size_t strsz);

// TEMP sprintf functions

int sprintf_1d(char* buffer, const char* format, uint64_t number);

char *strcpy( char *dest, const char *src );
char *strncpy( char *dest, const char *src, size_t count );

// My functions
char* strrev(char* str);

char* strdigits(uint64_t number);

#endif
