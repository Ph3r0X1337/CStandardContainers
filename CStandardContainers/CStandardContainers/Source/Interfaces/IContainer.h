#ifndef CSC_I_CONTAINER
#define CSC_I_CONTAINER

/*
Description:
The IContainer interface must be implemented by all containers in the CSC library that make use of allocators.
There might be containers that don't support all the operations that the interface provides, in that case the function pointer in the virtual table should be a null pointer.
Examples for this are associative arrays, which don't support typical insertion or removal of multiple elements based on an index or swapping values.
Another special property of this interface is the fact that the initialization method is part of the interface:
As container nesting is supported, operations that insert a default element are prohibited, hence insertions on such containers always require a valid element to be inserted.
The C language doesn't support RTTI internally, which makes constructing default elements impossible, as knowledge of the type is missing.
The IContainer interface requires an initializer method for a default element however, which is needed to copy the element to be inserted.
As the interface only requires the elements container type to match, making sure the underlying element type is matching is the responsibility of the user.
On the other hand, this opens the possibility for using varying types for the element types of the underlying containers.
However, this practice is highly discouraged and should only be utilized by experienced users.
The initialization method is called through the virtual table pointer, which is passed to the initializer of the container when container nesting is used.
*/

// Include the current configuration of the library containing various type definitions and other things.
#include "../Configuration/Configuration.h"
// Include the IAllocator interface as every container requires an allocator.
#include "IAllocator.h"

// Predecleration of the CSC_IContainer type to be able to create the function pointer types for the methods that the interface implements.
struct _CSC_IContainer;

// Type definitions of the function pointer types of the IContainer interfaces methods, which need to be implemented by types that utilize the interface.
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_CONTAINER_INIT)(_Out_ struct _CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CSC_IAllocator* CONST pIAllocator);
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_CONTAINER_ERASE)(_Inout_ struct _CSC_IContainer* CONST pThis);
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_CONTAINER_DESTROY)(_Inout_ struct _CSC_IContainer* CONST pThis);

typedef CSC_STATUS(CSCMETHOD* CSC_P_I_CONTAINER_COPY)(_Inout_ struct _CSC_IContainer* CONST pThis, _In_ CONST struct _CSC_IContainer* CONST pOther);
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_CONTAINER_MOVE)(_Inout_ struct _CSC_IContainer* CONST pThis, _Inout_ struct _CSC_IContainer* CONST pOther);

typedef CSC_STATUS(CSCMETHOD* CSC_P_I_CONTAINER_INSERT_RANGE)(_Inout_ struct _CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements);
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_CONTAINER_REMOVE_RANGE)(_Inout_ struct _CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements);
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_CONTAINER_SWAP_VALUES)(_Inout_ struct _CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T secondIndex);

typedef CSC_PVOID(CSCMETHOD* CSC_P_I_CONTAINER_ACCESS_ELEMENT)(_In_ CONST struct _CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T index);

typedef CSC_STATUS(CSCMETHOD* CSC_P_I_CONTAINER_IS_VALID)(_In_ CONST struct _CSC_IContainer* CONST pThis);
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_CONTAINER_IS_EMPTY)(_In_ CONST struct _CSC_IContainer* CONST pThis);
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_CONTAINER_IS_ELEMENT_CONTAINER)(_In_ CONST struct _CSC_IContainer* CONST pThis);

typedef CSC_SIZE_T(CSCMETHOD* CSC_P_I_CONTAINER_GET_SIZE)(_In_ CONST struct _CSC_IContainer* CONST pThis);
typedef CSC_SIZE_T(CSCMETHOD* CSC_P_I_CONTAINER_GET_ELEMENT_SIZE)(_In_ CONST struct _CSC_IContainer* CONST pThis);
typedef CSC_SIZE_T(CSCMETHOD* CSC_P_I_CONTAINER_GET_MAX_ELEMENTS)(_In_ CONST struct _CSC_IContainer* CONST pThis);
typedef CSC_IAllocator*(CSCMETHOD* CSC_P_I_CONTAINER_GET_I_ALLCATOR)(_In_ CONST struct _CSC_IContainer* CONST pThis);

// Definition of the virtual table layout of the IContainer type.
typedef struct _CSC_IContainerVirtualTable
{
	CSC_P_I_CONTAINER_INIT pInitialize;
	CSC_P_I_CONTAINER_ERASE pErase;
	CSC_P_I_CONTAINER_DESTROY pDestroy;
	CSC_P_I_CONTAINER_COPY pCopy;
	CSC_P_I_CONTAINER_MOVE pMove;
	CSC_P_I_CONTAINER_INSERT_RANGE pInsertRange;
	CSC_P_I_CONTAINER_REMOVE_RANGE pRemoveRange;
	CSC_P_I_CONTAINER_SWAP_VALUES pSwapValues;
	CSC_P_I_CONTAINER_ACCESS_ELEMENT pAccessElement;
	CSC_P_I_CONTAINER_IS_VALID pIsValid;
	CSC_P_I_CONTAINER_IS_EMPTY pIsEmpty;
	CSC_P_I_CONTAINER_IS_ELEMENT_CONTAINER pIsElementContainer;
	CSC_P_I_CONTAINER_GET_SIZE pGetSize;
	CSC_P_I_CONTAINER_GET_ELEMENT_SIZE pGetElementSize;
	CSC_P_I_CONTAINER_GET_MAX_ELEMENTS pGetMaxElements;
	CSC_P_I_CONTAINER_GET_I_ALLCATOR pGetIAllocator;
} CSC_IContainerVirtualTable;

// Definition of the IContainer type, contains a pointer to the virtual table of the object that implements the interfaces methods.
typedef struct _CSC_IContainer
{
	CSC_IContainerVirtualTable* pIContainerVirtualTable;
} CSC_IAllocator;

// Calls the underlying initialization routine to create the container containing a pure data type with the specified size.
// The method is invoked directly through the virtual table by the invoking (outer) container.
// It is used on element insertion prior to copying the inserted value, which is mandatory for insertion on nested containers.
// Nested containers are initialized with the allocator of the invoking (outer) container.
CSC_STATUS CSCMETHOD CSC_IContainerInitialize(_Out_ struct _CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CSC_IAllocator* CONST pIAllocator);
// Calls the underlying method to completely empty a container.
// This does not mean that the object will free allocated resources, therefore the Destroy method should be invoked to destruct a container.
CSC_STATUS CSCMETHOD CSC_IContainerErase(_Inout_ struct _CSC_IContainer* CONST pThis);
// Calls the underlying destructor for a container object, typically zeroing it out after resources were freed.
// Destructed containers must be initialized after this operation prior to using them again.
CSC_STATUS CSCMETHOD CSC_IContainerDestroy(_Inout_ struct _CSC_IContainer* CONST pThis);

// Calls the underlying copy method to perform a deep copy of the container, for this the container must match which is checked by comparing the virtual tables of both container, those must match.
CSC_STATUS CSCMETHOD CSC_IContainerCopy(_Inout_ struct _CSC_IContainer* CONST pThis, _In_ CONST struct _CSC_IContainer* CONST pOther);
// Calls the underlying move method to move the contents from another container to the current container.
// This does not destruct the source container, hence the source container might still contain unfreed resources.
CSC_STATUS CSCMETHOD CSC_IContainerMove(_Inout_ struct _CSC_IContainer* CONST pThis, _Inout_ struct _CSC_IContainer* CONST pOther);

// Calls the underlying method to perform an insertion of one or more elements.
// In the case of multiple elements, the elements need to be stored in an array / contiguous memory.
// Providing a nullptr for the pElements parameter is only allowed if the elements to be inserted are not containers themselves.
CSC_STATUS CSCMETHOD CSC_IContainerInsertRange(_Inout_ struct _CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements);
// Calls the underlying method to remove one or more contiguous elements from the container.
CSC_STATUS CSCMETHOD CSC_IContainerRemoveRange(_Inout_ struct _CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements);
// Calls the underlying method to swap the contents of two elements of the container based on their indices.
CSC_STATUS CSCMETHOD CSC_IContainerSwapValues(_Inout_ struct _CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T secondIndex);

// Calls the underlying method to access an element of the container in form of a CSC_PVOID pointing to the base address of the element.
CSC_PVOID CSCMETHOD CSC_IContainerAccessElement(_In_ CONST struct _CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T index);

// Calls the underlying method to check whether the container is in a valid state of operation.
// An invalid state can indicate a broken or uninitialized container, which needs to be repaired (if possible) or initialized.
CSC_STATUS CSCMETHOD CSC_IContainerIsValid(_In_ CONST struct _CSC_IContainer* CONST pThis);
// Calls the underlying method to check whether a container contains no elements.
CSC_STATUS CSCMETHOD CSC_IContainerIsEmpty(_In_ CONST struct _CSC_IContainer* CONST pThis);
// Calls the underlying method to check whether the elements contained by the container are containers themselves.
CSC_STATUS CSCMETHOD CSC_IContainerIsElementContainer(_In_ CONST struct _CSC_IContainer* CONST pThis);

// Calls the underlying method to get the number of elements that the container currently stores.
CSC_SIZE_T CSCMETHOD CSC_IContainerGetSize(_In_ CONST struct _CSC_IContainer* CONST pThis);
// Calls the underlying method to get the size in bytes that a container element consumes.
CSC_SIZE_T CSCMETHOD CSC_IContainerGetElementSize(_In_ CONST struct _CSC_IContainer* CONST pThis);
// Calls the underlying method to determine the maximum amount of elements the container can store with the given element size.
CSC_SIZE_T CSCMETHOD CSC_IContainerGetMaxElements(_In_ CONST struct _CSC_IContainer* CONST pThis);
// Calls the underlying method to retrieve the allocator that the container uses for memory management.
CSC_IAllocator* CSCMETHOD CSC_IContainerGetIAllocator(_In_ CONST struct _CSC_IContainer* CONST pThis);

#endif CSC_I_CONTAINER