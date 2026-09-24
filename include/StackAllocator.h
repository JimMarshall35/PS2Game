#ifndef STACK_ALLOCATOR_H
#define STACK_ALLOCATOR_H
#include <stddef.h>

enum SA_Stack
{
    SA_TopStack,
    SA_BottomStack
};

struct ResetMarker
{
    enum SA_Stack stack;
    void* ptr;
};

struct StackAllocator
{
    char name[64];
    size_t allocationAlignment;
    void* pMemoryAllocation;
    size_t allocationSize;
    void* pTopStackPtr;
    void* pBottomStackPtr;
};

void SA_InitStackAllocator(struct StackAllocator* pOutStack, size_t size, size_t allocationAlignment, const char* name);

void* SA_Allocate(struct StackAllocator* pStack, enum SA_Stack whichStack, struct ResetMarker* pMarker, size_t size);

void SA_ResetTo(struct StackAllocator* pStack, struct ResetMarker* pMarker);

#endif
