#ifndef FIXED_SIZE_OBJECT_POOL_H
#define FIXED_SIZE_OBJECT_POOL_H

#include "IntTypes.h"

typedef struct ObjectPool
{
    u8* storage;      // raw backing buffer
    struct FreeNode* free_list;   // head of free list, NULL if pool exhausted
    u32 object_size;    // size of one slot (must be >= sizeof(FreeNode))
    u32 capacity;
} ObjectPool;

void PoolInit(ObjectPool* pool, u8* storage, u32 object_size, u32 capacity);

void* PoolAlloc(ObjectPool* pool);

void PoolFree(ObjectPool* pool, void* ptr);

#endif