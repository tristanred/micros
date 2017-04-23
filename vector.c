#include "vector.h"
#include "memory.h"

#include "common.h"

vector* vector_create()
{
    return vector_create_size(10);
}

// Vector public interface
vector* vector_create_size(size_t size)
{
    vector* vec = kmKernelAlloc(sizeof(vector));
    
    vec->size = size;
    vec->count = 0;
    vec->dataElements = kmKernelAlloc(size * sizeof(void*));
    
    return vec;
}

void vector_add(vector* vec, void* element)
{
    if(vec == NULL)
        return;
        
    // Check for element == NULL ?
    
    if(vec->count >= vec->size)
    {
        vector_resize(vec, vec->size * 2);
    }
    
    vec->dataElements[vec->count] = element;
    vec->count++;
}

void vector_insert(vector* vec, size_t index, void* element)
{
    if(vec == NULL)
        return;

    if(vec->count + 1 > vec->size == FALSE)
    {
        vector_resize(vec, vec->size*2);
    }
    
    vector_shift_right(vec, index);
    
    vec->dataElements[index] = element;
}

void vector_remove(vector* vec, void* element)
{
    if(vec == NULL)
        return;
    
    for(size_t i = 0; i < vec->count; i++)
    {
        if(vec->dataElements[i] == element)
        {
            vector_shift_left(vec, i + 1);
            vec->count--;
            
            return;
        }
    }
}

void vector_remove_at(vector* vec, size_t index)
{
    if(vec == NULL)
        return;
    
    if(index >= vec->size)
        return;
    
    vector_shift_left(vec, index + 1);
    vec->count--;
}

void* vector_get_at(vector* vec, size_t index)
{
    if(vec == NULL)
        return NULL;
    
    if(index >= vec->size)
        return NULL;
    
    return vec->dataElements[index];
}

// Vector management methods
void vector_resize(vector* vec, size_t newSize)
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
        void* newData = kmKernelAlloc(newSize);
        kmKernelCopy(vec->dataElements, newData);
        
        kmKernelFree(vec->dataElements);
        vec->dataElements = newData;
    }
}

void vector_shift_left(vector* vec, size_t startIndex)
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

void vector_shift_right(vector* vec, size_t startIndex)
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
