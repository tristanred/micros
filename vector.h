#ifndef vector_h
#define vector_h

#include <stdint.h>
#include <stddef.h>

// Kernel vector structure. This structure uses the kernel heap to allocate
// memory. Can ONLY be used in kernel mode.
// Can technically stack allocate data but the struct will use the heap when
// resizing the vector.
struct vector {
    size_t size; // Size of the array's memory
    
    size_t count; // Count of the elements in the array
    uint32_t* dataElements;
};

// Vector public interface
struct vector* vector_create();
struct vector* vector_create_size(size_t size);

// This sets up a local vector BUT this does not create the array to store
// elements. So the first allocation will cause a resize. TODO : Need to handle
// that case.
void vector_setup(struct vector* vec);

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
