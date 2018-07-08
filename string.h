#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "types.h"

// C LIBRARY FUNCTIONS
size_t strlen(const char* str);
size_t strlen_s(const char* str, size_t strsz);

// TEMP sprintf functions
int sprintf( char *buffer, const char *format, ... );
int vsprintf( char *buffer, const char *format, va_list vlist );

char *strcpy( char *dest, const char *src );
char *strncpy( char *dest, const char *src, size_t count );

int strcmp( const char *lhs, const char *rhs );
int strncmp( const char *lhs, const char *rhs, size_t count );

void *memset( void *dest, int ch, size_t count );

// My functions

// Compare count bytes of memory.
BOOL mcmp(uint8_t* lhs, uint8_t* rhs, size_t count);

// Split a string into multiple strings separated by 'separator'
char** strspl(char* buffer, char* separator, size_t* amount);

// Free the parts of string returned by 'strspl'
void splfree(char** splitParts, size_t parts);

// Reverse a string
void strrev(char* str, char* out);

// Convert a number to a string
void strdigits(uint64_t number, char* buf);

// Convert a string to a number
int atoi( const char *str );

#endif
