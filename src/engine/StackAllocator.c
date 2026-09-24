/*
    Copyright Jim Marshall 2026.
    File: StackAllocator.c
    Description: A double ended stack allocator. Each instance of the stack allocator is two stack allocators in one:
    one grows from the top down and one from the bottom up. Allocation function returns a marker which can be used
    to reset the stacks to the state they were at before that allocation, deallocating the allocation and all subsequent ones
    for th that stack
*/
/////////////////////////////////////////////////////////////////////////////////////////// Standard Library Includes

#include <string.h>

/////////////////////////////////////////////////////////////////////////////////////////// SDK Includes

/////////////////////////////////////////////////////////////////////////////////////////// Third Party Includes

/////////////////////////////////////////////////////////////////////////////////////////// First Party Includes

#include "StackAllocator.h"
#include "Memory.h"
#include "Log.h"

/////////////////////////////////////////////////////////////////////////////////////////// Typedefs

/////////////////////////////////////////////////////////////////////////////////////////// Defines

/////////////////////////////////////////////////////////////////////////////////////////// Enums

/////////////////////////////////////////////////////////////////////////////////////////// Structs

/////////////////////////////////////////////////////////////////////////////////////////// Private Globals

/////////////////////////////////////////////////////////////////////////////////////////// Public Globals

/////////////////////////////////////////////////////////////////////////////////////////// Private Functions


/////////////////////////////////////////////////////////////////////////////////////////// Public Functions

void SA_InitStackAllocator(struct StackAllocator* pOutStack, size_t size, size_t allocationAlignment, const char* name)
{
    strcpy(pOutStack->name, name);
    pOutStack->pMemoryAllocation = Mem_malloc(size, name);
    pOutStack->allocationSize = size;
    pOutStack->allocationAlignment = allocationAlignment;
    pOutStack->pBottomStackPtr = pOutStack->pMemoryAllocation;
    Mem_AlignUp(pOutStack->pBottomStackPtr, allocationAlignment);
    pOutStack->pTopStackPtr = (char*)pOutStack->pBottomStackPtr + pOutStack->allocationSize;
    pOutStack->pTopStackPtr = Mem_AlignDown(pOutStack->pTopStackPtr, allocationAlignment);

}

void* SA_Allocate(struct StackAllocator* pStack, enum SA_Stack whichStack, struct ResetMarker* pMarker, size_t size)
{
    pMarker->stack = whichStack;
    void* pOut = NULL;
    switch (whichStack)
    {
    case SA_TopStack:
        {
            void* ptrAfterAlloc = (char*)pStack->pTopStackPtr - size;
            ptrAfterAlloc = Mem_AlignDown(ptrAfterAlloc, pStack->allocationAlignment);
            if(ptrAfterAlloc < pStack->pBottomStackPtr)
            {
                Log_Error("Stack %s sub stack %i is out of memory! allocation of size %i failed", pStack->name, whichStack, size);
                return NULL;
            }
            pMarker->ptr = pStack->pTopStackPtr;
            pStack->pTopStackPtr = ptrAfterAlloc;
            pOut = pStack->pTopStackPtr;
        }
        break;
    case SA_BottomStack:
        {
            void* ptrAfterAlloc = (char*)pStack->pBottomStackPtr + size;
            ptrAfterAlloc = Mem_AlignUp(ptrAfterAlloc, pStack->allocationAlignment);
            if(ptrAfterAlloc > pStack->pTopStackPtr)
            {
                Log_Error("Stack %s sub stack %i is out of memory! allocation of size %i failed", pStack->name, whichStack, size);
                return NULL;
            }
            pOut = pStack->pBottomStackPtr;
            pMarker->ptr = pOut;
            pStack->pBottomStackPtr = ptrAfterAlloc;
        }
        break;
    }
    return pOut;
}

void SA_ResetTo(struct StackAllocator* pStack, struct ResetMarker* pMarker)
{
    switch (pMarker->stack)
    {
    case SA_TopStack:
        pStack->pTopStackPtr = pMarker->ptr;
        break;
    case SA_BottomStack:
        pStack->pBottomStackPtr = pMarker->ptr;
        break;
    }
}

