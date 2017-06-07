#ifndef vector_h
#define vector_h

#include <stdint.h>
#include <stddef.h>

struct vector {
    size_t size;
    
    size_t count;
    uint32_t* dataElements;
};

// Vector public interface
struct vector* vector_create();
struct vector* vector_create_size(size_t size);

void vector_add(struct vector* vec, void* element);

void vector_insert(struct vector* vec, size_t index, void* element);

void vector_remove(struct vector* vec, void* element);
void vector_remove_at(struct vector* vec, size_t index);

void* vector_get_at(struct vector* vec,size_t index);

void** vector_get_array(struct vector* vec, int* count);

// Vector management methods
void vector_resize(struct vector* vec, size_t newSize);
void vector_shift_left(struct vector* vec, size_t startIndex);
void vector_shift_right(struct vector* vec, size_t startIndex);

#endif
