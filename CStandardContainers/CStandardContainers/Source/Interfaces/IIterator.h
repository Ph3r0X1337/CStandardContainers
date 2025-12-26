#ifndef CSC_I_ITERATOR
#define CSC_I_ITERATOR

/*
Description:
The IIterator interface must be implemented by all iterator objects that wish to iterate over the iterables of the CSC library.
The iterable uses the IIterator interface as a observer pattern, to notify the subscribed iterator objects on any updates that occurr during changes in the iterable.
While the IIterator interface methods could be directly called on the iterator by an arbitrary caller, this use is discouraged unless there is a justified need for it.
Instead it is encouraged to invoke the methods of the object implementing the IIterator interface to make changes to the iterator.
*/

// Include the current configuration of the library containing various type definitions and other things.
#include "../Configuration/Configuration.h"
// Include the string utilities which define the invalid string length, which is used to define the invalid length and index of an iteration.
#include "../Utils/StringUtils.h"

#define CSC_ITERATOR_INVALID_LENGTH CSC_STRING_INVALID_LENGTH
#define CSC_ITERATOR_INVALID_INDEX CSC_ITERATOR_INVALID_LENGTH

// Predecleration of the CSC_IIterator type to be able to create the function pointer types for the methods that the interface implements.
struct _CSC_IIterator;

// Type definitions of the function pointer types of the IIterator interfaces methods, which need to be implemented by types that utilize the interface.
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_ITERATOR_INVALIDATE_ITERATION)(_Inout_ struct _CSC_IIterator* CONST pThis);
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_ITERATOR_UPDATE_ITERATION)(_Inout_ struct _CSC_IIterator* CONST pThis);

typedef CSC_STATUS(CSCMETHOD* CSC_P_I_ITERATOR_ON_INSERTION)(_Inout_ struct _CSC_IIterator* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T newSize);
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_ITERATOR_ON_REMOVAL)(_Inout_ struct _CSC_IIterator* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T newSize);
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_ITERATOR_ON_DESTRUCTION)(_Inout_ struct _CSC_IIterator* CONST pThis);

// Definition of the virtual table layout of the IIterator type.
typedef struct _CSC_IIteratorVirtualTable
{
	CSC_P_I_ITERATOR_INVALIDATE_ITERATION pInvalidateIteration;
	CSC_P_I_ITERATOR_UPDATE_ITERATION pUpdateIteration;
	CSC_P_I_ITERATOR_ON_INSERTION pOnInsertion;
	CSC_P_I_ITERATOR_ON_REMOVAL pOnRemoval;
	CSC_P_I_ITERATOR_ON_DESTRUCTION pOnDestruction;
} CSC_IIteratorVirtualTable;

// Definition of the IIterator type, contains a pointer to the virtual table of the object that implements the interfaces methods.
typedef struct _CSC_IIterator
{
	CSC_IIteratorVirtualTable* pIIteratorVirtualTable;
} CSC_IIterator;

// Calls the underlying method to invalidate the iteration, if it is currently valid.
// Should return CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCMETHOD CSC_IIteratorInvalidateIteration(_Inout_ CSC_IIterator* CONST pThis);
// Calls the underlying method to update the iteration, if it is currently valid.
// This method is usually called when the iterables internal resources have changed, which results in a need to update pointers to elements within the iterable.
// Should return CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCMETHOD CSC_IIteratorUpdateIteration(_Inout_ CSC_IIterator* CONST pThis);

// Calls the underlying method to notify the iterator on a insertion of elements in the iterable, which is typical when the iterable is also a container.
// Should return CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCMETHOD CSC_IIteratorOnInsertion(_Inout_ CSC_IIterator* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T newSize);
// Calls the underlying method to notify the iterator on a removal of elements in the iterable, which is typical when the iterable is also a container.
// Should return CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCMETHOD CSC_IIteratorOnRemoval(_Inout_ CSC_IIterator* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T newSize);
// Calls the underlying method to indicate the destruction of the iterable object.
// The iterable object should unregister the bound iterator itself, hence this method should not invoke unregistration on the iterable.
// Should return CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCMETHOD CSC_IIteratorOnDestruction(_Inout_ CSC_IIterator* CONST pThis);

#endif