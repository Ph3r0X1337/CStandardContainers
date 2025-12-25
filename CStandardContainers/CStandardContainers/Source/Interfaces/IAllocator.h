#ifndef CSC_I_ALLOCATOR
#define CSC_I_ALLOCATOR

/*
Description:
The IAllocator interface must be implemented by allocators that are supposed to be used with the CSC library.
Most objects in the CSC library require an allocator, which will call into the generic methods of the interface.
Hence all of the methods must be implemented, especially the AllocZero method, which is almost exclusively used for internal allocations.
If such a primitive does not exist for a given allocator, it can be easily implemented through combining a basic allocation primitive with the MemoryUtilsZeroMemory function.
*/

// Include the current configuration of the library containing various type definitions and other things.
#include "../Configuration/Configuration.h"

// Predecleration of the CSC_IAllocator type to be able to create the function pointer types for the methods that the interface implements.
struct _CSC_IAllocator;

// Type definitions of the function pointer types of the IAllocator interfaces methods, which need to be implemented by types that utilize the interface.
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_ALLOCATOR_INIT)(_Inout_ struct _CSC_IAllocator* CONST pThis);
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_ALLOCATOR_CLEANUP)(_Inout_ struct _CSC_IAllocator* CONST pThis);

typedef CSC_PVOID(CSCMETHOD* CSC_P_I_ALLOCATOR_ALLOC)(_In_ CONST struct _CSC_IAllocator* CONST pThis, _In_ CONST CSC_SIZE_T size);
typedef CSC_PVOID(CSCMETHOD* CSC_P_I_ALLOCATOR_ALLOC_ZERO)(_In_ CONST struct _CSC_IAllocator* CONST pThis, _In_ CONST CSC_SIZE_T size);

typedef CSC_STATUS(CSCMETHOD* CSC_P_I_ALLOCATOR_FREE)(_In_ CONST struct _CSC_IAllocator* CONST pThis, _In_ CONST CSC_PVOID pMemoryBlock);

typedef CSC_STATUS(CSCMETHOD* CSC_P_I_ALLOCATOR_IS_USABLE)(_In_ CONST struct _CSC_IAllocator* CONST pThis);

// Definition of the virtual table layout of the IAllocator type.
typedef struct _CSC_IAllocatorVirtualTable
{
	CSC_P_I_ALLOCATOR_INIT pInitialize;
	CSC_P_I_ALLOCATOR_CLEANUP pCleanup;
	CSC_P_I_ALLOCATOR_ALLOC pAlloc;
	CSC_P_I_ALLOCATOR_ALLOC_ZERO pAllocZero;
	CSC_P_I_ALLOCATOR_FREE pFree;
	CSC_P_I_ALLOCATOR_IS_USABLE pIsUsable;
} CSC_IAllocatorVirtualTable;

// Definition of the IAllocator type, contains a pointer to the virtual table of the object that implements the interfaces methods.
typedef struct _CSC_IAllocator
{
	CSC_IAllocatorVirtualTable* pIAllocatorVirtualTable;
} CSC_IAllocator;

// Calls the underlying initialization method implemented by the allocator object.
// Should return CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCMETHOD CSC_IAllocatorInit(_Inout_ struct _CSC_IAllocator* CONST pThis);
// Calls the underlying cleanup method implemented by the allocator object.
// Should return CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCMETHOD CSC_IAllocatorCleanup(_Inout_ struct _CSC_IAllocator* CONST pThis);

// Calls the underlying allocation method implemented by the allocator object.
// Should return a pointer to the allocated memory on success, otherwise a nullptr should be returned.
CSC_PVOID CSCMETHOD CSC_IAllocatorAlloc(_In_ CONST struct _CSC_IAllocator* CONST pThis, _In_ CONST CSC_SIZE_T size);
// Calls the underlying allocation and zero memory method implemented by the allocator object.
// Should return a pointer to the allocated memory on success, otherwise a nullptr should be returned.
CSC_PVOID CSCMETHOD CSC_IAllocatorAllocZero(_In_ CONST struct _CSC_IAllocator* CONST pThis, _In_ CONST CSC_SIZE_T size);

// Calls the underlying free method implemented by the allocator object to free a priorly allocated memory block.
// Should return CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCMETHOD CSC_IAllocatorFree(_In_ CONST struct _CSC_IAllocator* CONST pThis, _In_ CONST CSC_PVOID pMemoryBlock);

// Calls the underlying method implemented by the allocator object to check if the allocator is in a usable state.
// Should return true or false depending on the situation.
CSC_BOOLEAN CSCMETHOD CSC_IAllocatorIsUsable(_In_ CONST struct _CSC_IAllocator* CONST pThis);

#endif CSC_I_ALLOCATOR