/*
    Copyright Jim Marshall 2026.
    File: FixedSizeObjectPool.c
    Description: A fixed size object pool.
*/
/////////////////////////////////////////////////////////////////////////////////////////// Standard Library Includes

#include <stdint.h>
#include <stddef.h>

/////////////////////////////////////////////////////////////////////////////////////////// First Party Includes

#include "FixedSizeObjectPool.h"

/////////////////////////////////////////////////////////////////////////////////////////// Structs

typedef struct FreeNode
{
    struct FreeNode* next;
} FreeNode;

/////////////////////////////////////////////////////////////////////////////////////////// Public Functions

void PoolInit(ObjectPool* pool, u8* storage, u32 object_size, u32 capacity)
{
    // enforce slot size can hold a free-list pointer
    if (object_size < sizeof(FreeNode))
    {
        object_size = sizeof(FreeNode);
    }

    pool->storage = storage;
    pool->object_size = object_size;
    pool->capacity = capacity;

    // thread the free list through every slot, last -> NULL
    pool->free_list = NULL;
    for (size_t i = capacity; i-- > 0; )
    {
        FreeNode* node = (FreeNode*)(storage + i * object_size);
        node->next = pool->free_list;
        pool->free_list = node;
    }
}

void* PoolAlloc(ObjectPool* pool)
{
    if (!pool->free_list)
        return NULL;  // exhausted

    FreeNode* node = pool->free_list;
    pool->free_list = node->next;
    return (void*)node;
}

void PoolFree(ObjectPool* pool, void* ptr)
{
    FreeNode* node = (FreeNode*)ptr;
    node->next = pool->free_list;
    pool->free_list = node;
}