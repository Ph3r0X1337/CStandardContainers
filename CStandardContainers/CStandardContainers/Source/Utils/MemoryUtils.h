#ifndef CSC_MEMORY_UTILS
#define CSC_MEMORY_UTILS

/*
Description:
The MemoryUtils provide a rich variaty of helper functions to perform operations like copying, moving, setting and comparing data in memory.
Most of the functions behave differently based on the configuration, which can decide whether they take advantage of size based optimization which also allows utilizing recursion.
However at the current state of the library size based optimization implies that misaligned access for such types is supported by the architecture, which affects all copy operations.
For special needs the MemoryUtils additionally expose the *Basic functions, which can be used to explicitly use unoptimized code performing byte-by-byte operations, even when size based optimization is enabled.
*/

// Include the current configuration of the library containing various type definitions and other things.
#include "../Configuration/Configuration.h"
// Include the IAllocator interface as some of the swap functions require dynamic memory allocation.
#include "../Interfaces/IAllocator.h"

// Helper function for copying memory from a source to a destination and a given size byte-by-byte.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsCopyMemoryBasic(_When_(return == CSC_STATUS_SUCCESS, _Out_) CONST CSC_PVOID pDst, _In_ CONST CSC_PCVOID pSrc, _In_ CONST CSC_SIZE_T size);
// Helper function for setting the value of bytes in contiguous memory at the destination and a given size byte-by-byte.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsSetMemoryBasic(_When_(return == CSC_STATUS_SUCCESS, _Out_) CONST CSC_PVOID pDst, _In_ CONST CSC_BYTE value, _In_ CONST CSC_SIZE_T size);
// Helper function for zeroing out contiguous memory at the destination and a given size byte-by-byte.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsSetZeroMemoryBasic(_Out_ CONST CSC_PVOID pDst, _In_ CONST CSC_SIZE_T size);
// Helper function for setting the value of elements in contiguous memory at the destination and a given element count byte-by-byte.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsSetArrayValueBasic(_Out_ CONST CSC_PVOID pDst, _In_ CONST CSC_PCVOID pValue, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T elementCount);

// Helper function for comparing the values of bytes in two locations with a given size byte-by-byte.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsCompareMemoryBasic(_In_ CONST CSC_PCVOID pFirst, _In_ CONST CSC_PCVOID pSecond, _In_ CONST CSC_SIZE_T size);

// Helper function for copying memory from a source to a destination and a given size.
// Depending on configuration this function may take advantage of copying in larger chunks depending on the configuration.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsCopyMemory(_Out_ CONST CSC_PVOID pDst, _In_ CONST CSC_PCVOID pSrc, _In_ CONST CSC_SIZE_T size);
// Helper function for setting the value of bytes in contiguous memory at the destination and a given size.
// Depending on configuration this function may take advantage of setting the value in larger chunks depending on the configuration.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsSetMemory(_Out_ CONST CSC_PVOID pDst, _In_ CONST CSC_BYTE value, _In_ CONST CSC_SIZE_T size);
// Helper function for zeroing out contiguous memory at the destination and a given size.
// Depending on configuration this function may take advantage of zeroing memory in larger chunks depending on the configuration.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsSetZeroMemory(_Out_ CONST CSC_PVOID pDst, _In_ CONST CSC_SIZE_T size);
// Helper function for setting the value of elements in contiguous memory at the destination and a given element count.
// Depending on configuration this function may take advantage of setting values in larger chunks depending on the configuration.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsSetArrayValue(_Out_ CONST CSC_PVOID pDst, _In_ CONST CSC_PCVOID pValue, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T elementCount);

// Helper function for comparing the values of bytes in two locations with a given size.
// Depending on configuration this function may take advantage of comparing memory in larger chunks depending on the configuration.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsCompareMemory(_In_ CONST CSC_PCVOID pFirst, _In_ CONST CSC_PCVOID pSecond, _In_ CONST CSC_SIZE_T size);

// Helper function for swapping the values stored in memory in two locations with a given size.
// Depending on configuration this function may take advantage of copying memory in larger chunks depending on the configuration.
// For a size greater than 256 bytes the function will utilize the allocator for creating a dynamically sized buffer for the swap.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsSwapValues(_Inout_ CONST CSC_PVOID pFirst, _Inout_ CONST CSC_PVOID pSecond, _In_ CONST CSC_SIZE_T size, _In_ CONST CSC_IAllocator* CONST pIAllocator);
// Helper function for swapping the values stored in memory in two locations with a given size.
// Depending on configuration this function may take advantage of copying memory in larger chunks depending on the configuration.
// The function will utilize the allocator for creating a dynamically sized buffer for the swap.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsSwapValuesHeap(_Inout_ CONST CSC_PVOID pFirst, _Inout_ CONST CSC_PVOID pSecond, _In_ CONST CSC_SIZE_T size, _In_ CONST CSC_IAllocator* CONST pIAllocator);
// Helper function for swapping the values stored in memory in two locations with a given size.
// Depending on configuration this function may take advantage of copying memory in larger chunks depending on the configuration.
// Only sizes up to 256 bytes are allowed as this function uses a stack based buffer to perform the swap.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsSwapValues256(_Inout_ CONST CSC_PVOID pFirst, _Inout_ CONST CSC_PVOID pSecond, _In_ CONST CSC_SIZE_T size);
// Helper function for moving a value stored in memory from source to destination with a given size.
// Depending on configuration this function may take advantage of copying and zeroing memory in larger chunks depending on the configuration.
// The function zeroes out the memory at the source after the value has been successfully copied to the destination.
// Overlapping move operations are supported by the function.
// Returns CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCAPI CSC_MemoryUtilsMoveValue(_Out_ CONST CSC_PVOID pDst, _Inout_ CONST CSC_PVOID pSrc, _In_ CONST CSC_SIZE_T size);

#endif
