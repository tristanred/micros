#ifndef vector_h
#define vector_h

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    size_t size;
    
    size_t count;
    uint32_t* dataElements;
} vector;

// Vector public interface
vector* vector_create();
vector* vector_create_size(size_t size);

void vector_add(vector* vec, void* element);

void vector_insert(vector* vec, size_t index, void* element);

void vector_remove(vector* vec, void* element);
void vector_remove_at(vector* vec, size_t index);

void* vector_get_at(vector* vec,size_t index);

// Vector management methods
void vector_resize(vector* vec, size_t newSize);
void vector_shift_left(vector* vec, size_t startIndex);
void vector_shift_right(vector* vec, size_t startIndex);

#endif
