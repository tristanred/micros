#include "vector.h"
#include "memory.h"

#include "common.h"

struct vector* vector_create()
{
    return vector_create_size(10);
}

// Vector public interface
struct vector* vector_create_size(size_t size)
{
    struct vector* vec = kmalloc(sizeof(struct vector));
    
    vec->size = size;
    vec->count = 0;
    vec->dataElements = kmalloc(size * sizeof(void*));
    
    return vec;
}

void vector_setup(struct vector* vec)
{
    vec->size = 0;
    vec->count = 0;
    vec->dataElements = NULL;
}

void vector_add(struct vector* vec, void* element)
{
    if(vec == NULL)
        return;
        
    // Check for element == NULL ?
    
    if(vec->count >= vec->size)
    {
        vector_resize(vec, vec->size * 2);
    }
    
    vec->dataElements[vec->count] = (uint32_t)element;
    vec->count++;
}

void vector_insert(struct vector* vec, size_t index, void* element)
{
    if(vec == NULL)
        return;

    if((vec->count + 1 > vec->size) == FALSE)
    {
        vector_resize(vec, vec->size*2);
    }
    
    vector_shift_right(vec, index);
    
    vec->dataElements[index] = (uint32_t)element;
}

void vector_remove(struct vector* vec, void* element)
{
    if(vec == NULL)
        return;
    
    for(size_t i = 0; i < vec->count; i++)
    {
        if(vec->dataElements[i] == (uint32_t)element)
        {
            vector_shift_left(vec, i + 1);
            vec->count--;
            
            return;
        }
    }
}

void vector_remove_at(struct vector* vec, size_t index)
{
    if(vec == NULL)
        return;
    
    if(index >= vec->size)
        return;
    
    vector_shift_left(vec, index + 1);
    vec->count--;
}

void* vector_get_at(struct vector* vec, size_t index)
{
    if(vec == NULL)
        return NULL;
    
    if(index >= vec->count)
        return NULL;
    
    return (void*)vec->dataElements[index];
}

void** vector_get_array(struct vector* vec, int* count)
{
    *count = vec->count;
    
    return (void**)vec->dataElements;
}

BOOL vector_exists(struct vector* vec, void* element)
{
    if(vec == NULL)
        return FALSE;
    
    for(size_t i = 0; i < vec->count; i++)
    {
        if(vec->dataElements[i] == (uint32_t)element)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

// Vector management methods
void vector_resize(struct vector* vec, size_t newSize)
{
    if(vec == NULL)
        return;
    
    if(newSize <= vec->count)
        return;
    
    if(newSize > vec->count && newSize < vec->size)
    {
        // Shrink array
    }
    else
    {
        // TODO : Implement realloc
        void* newData = kmalloc(newSize);
        
        memcpy(newData, vec->dataElements, vec->count * sizeof(uint32_t));
        kfree(vec->dataElements);
        vec->dataElements = newData;
    }
}

void vector_shift_left(struct vector* vec, size_t startIndex)
{
    if(vec == NULL)
        return;
    
    if(startIndex <= 0 || startIndex >= vec->size)
        return;
        
    for(size_t i = startIndex - 1; i < vec->size; i++)
    {
        vec->dataElements[i] = vec->dataElements[i + 1];
    }
    
    vec->dataElements[vec->count] = 0;
    
    // Secondary implementation with memcopy
    //kmKernelCopy(vec->dataElements[startIndex], vec->dataElements[startIndex - 1]);
}

void vector_shift_right(struct vector* vec, size_t startIndex)
{
    if(vec == NULL)
        return;
    
    if(startIndex >= vec->size || vec->count + 1 < vec->size)
        return;
        
    for(size_t i = vec->count + 1; i > startIndex; i--)
    {
        vec->dataElements[i] = vec->dataElements[i - 1];
    }
    
    // Secondary implementation with memcopy
    //kmKernelCopy(vec->dataElements[startIndex], vec->dataElements[startIndex + 1]);
}
