#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>
#include "types.h"

// C LIBRARY FUNCTIONS
size_t strlen(const char* str);
size_t strlen_s(const char* str, size_t strsz);

// TEMP sprintf functions
int sprintf_1d(char* buffer, const char* format, uint64_t number);

char *strcpy( char *dest, const char *src );
char *strncpy( char *dest, const char *src, size_t count );

int strcmp( const char *lhs, const char *rhs );
int strncmp( const char *lhs, const char *rhs, size_t count );

void *memset( void *dest, int ch, size_t count );

// My functions

// Self allocating sprintf function
char* alloc_sprintf_1d(char* buffer, const char* format, uint64_t number, int* nbWritten);

BOOL mcmp(uint8_t* lhs, uint8_t* rhs, size_t count);

// Split a string into multiple strings separated by 'separator'
char** strspl(char* buffer, char* separator, size_t* amount);

void splfree(char** splitParts, size_t parts);

// Reverse a string
char* strrev(char* str);

// Convert a number to a string
char* strdigits(uint64_t number);

uint64_t s_to_d(char* number);

// Stack allocated functions
int sprintf_1d_buf(char* buffer, const char* format, uint64_t number);
void strdigits_buf(uint64_t number, char* buf);
void strrev_buf(char* str, char* buf);

#endif
