#ifndef ENGINE_MEMORY_H
#define ENGINE_MEMORY_H
#include <stddef.h>

struct MemoryAllocation
{
    /* data */
    char purpose[64];
    size_t size;
    void* pAddress;
};


/*
    No part of the engine will use a raw malloc call, instead this will be used.
    There is intentionally no free, parts of the game and engine will allocate memory in large chunks and manage
    data within those chunks, which are all of a fixed size.
*/
void* Mem_malloc(size_t size, const char* purpose);

/// @brief Get the number of top level memory allocations
/// @return the number of memory allocations
size_t Mem_GetNumAllocations();

/// @brief Get information about a memory allocation
/// @return NULL if index out of range or a MemoryAllocation pointer
struct MemoryAllocation* Mem_GetAllocationInfo(size_t i);

/// @brief Align a pointer upwards
/// @param ptr 
/// @param align must be a power of 2
/// @return 
void* Mem_AlignUp(void* ptr, size_t align);

/// @brief Align a pointer downwards
/// @param ptr 
/// @param align must be a power of 2
/// @return 
void* Mem_AlignDown(void* ptr, size_t align);

#endif