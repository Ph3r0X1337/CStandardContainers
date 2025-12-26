#ifndef CSC_I_ITERABLE
#define CSC_I_ITERABLE

/*
Description:
The IIterable interface must be implemented by all iterable objects in the CSC library that wish to be iterable through the IIterable interface.
The interface is usually invoked by an object that supports the IIterator interface, hence arbitrary invocations are discouraged.
Instead the dedicated methods of the objects that implement the IIterator interface should be used to traverse the IIterable object.
*/

// Include the current configuration of the library containing various type definitions and other things.
#include "../Configuration/Configuration.h"
// Include the IIterator interface as the IIterable interface requires working with IIterator.
#include "IIterator.h"

// Predecleration of the CSC_IIterable type to be able to create the function pointer types for the methods that the interface implements.
struct _CSC_IIterable;

// Type definitions of the function pointer types of the IIterable interfaces methods, which need to be implemented by types that utilize the interface.
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_ITERABLE_REGISTER_ITERATOR)(_Inout_ struct _CSC_IIterable* CONST pThis, _In_ CONST CSC_IIterator* CONST pIIterator);
typedef CSC_STATUS(CSCMETHOD* CSC_P_I_ITERABLE_UNREGISTER_ITERATOR)(_Inout_ struct _CSC_IIterable* CONST pThis);

typedef CSC_PVOID(CSCMETHOD* CSC_P_I_ITERABLE_FIRST_ELEMENT)(_In_ CONST struct _CSC_IIterable* CONST pThis);
typedef CSC_PVOID(CSCMETHOD* CSC_P_I_ITERABLE_NEXT_ELEMENT)(_In_ CONST struct _CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement);
typedef CSC_PVOID(CSCMETHOD* CSC_P_I_ITERABLE_LAST_ELEMENT)(_In_ CONST struct _CSC_IIterable* CONST pThis);
typedef CSC_PVOID(CSCMETHOD* CSC_P_I_ITERABLE_PREVIOUS_ELEMENT)(_In_ CONST struct _CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement);

typedef CSC_PVOID(CSCMETHOD* CSC_P_I_ITERABLE_GET_ELEMENT_AT)(_In_ CONST struct _CSC_IIterable* CONST pThis, _In_ CONST CSC_SIZE_T index, _In_opt_ CONST CSC_SIZE_T currentIndex, _In_opt_ CONST CSC_PVOID pCurrentElement);
typedef CSC_SIZE_T(CSCMETHOD* CSC_P_I_ITERABLE_GET_ELEMENT_COUNT)(_In_ CONST struct _CSC_IIterable* CONST pThis);

// Definition of the virtual table layout of the IIterable type.
typedef struct _CSC_IIterableVirtualTable
{
	CSC_P_I_ITERABLE_REGISTER_ITERATOR pRegisterIterator;
	CSC_P_I_ITERABLE_UNREGISTER_ITERATOR pUnregisterIterator;
	CSC_P_I_ITERABLE_FIRST_ELEMENT pFirstElement;
	CSC_P_I_ITERABLE_NEXT_ELEMENT pNextElement;
	CSC_P_I_ITERABLE_LAST_ELEMENT pLastElement;
	CSC_P_I_ITERABLE_PREVIOUS_ELEMENT pPreviousElement;
	CSC_P_I_ITERABLE_GET_ELEMENT_AT pGetElementAt;
	CSC_P_I_ITERABLE_GET_ELEMENT_COUNT pGetElementCount;
} CSC_IIterableVirtualTable;

// Definition of the IIterable type, contains a pointer to the virtual table of the object that implements the interfaces methods.
typedef struct _CSC_IIterable
{
	CSC_IIterableVirtualTable* pIIterableVirtualTable;
} CSC_IIterable;

// Calls the underlying method on the IIterable to register an observing IIterator object, which is typically the object that invokes the method.
// Should return CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCMETHOD CSC_IIterableRegisterIterator(_Inout_ CSC_IIterable* CONST pThis, _In_ CONST CSC_IIterator* CONST pIIterator);
// Calls the underlying method on the IIterable to unregister an observing IIterator object, which is typically the object that invokes the method.
// Should return CSC_STATUS_SUCCESS on success, otherwise an error code is returned.
CSC_STATUS CSCMETHOD CSC_IIterableUnregisterIterator(_Inout_ CSC_IIterable* CONST pThis);

// Calls the underlying method on the IIterable to retrieve the first element of the IIterable object, typically is invoked by an IIterator object.
// Should return a pointer to the requested element on success, otherwise a null pointer is returned.
CSC_PVOID CSCMETHOD CSC_IIterableFirstElement(_In_ CONST CSC_IIterable* CONST pThis);
// Calls the underlying method on the IIterable to retrieve the next element of the IIterable object, typically is invoked by an IIterator object.
// The method needs the invoker to pass the current index aswell as a pointer to the current element of the iteration, so that the IIterable can find the next element if it exists.
// Should return a pointer to the requested element on success, otherwise a null pointer is returned.
CSC_PVOID CSCMETHOD CSC_IIterableNextElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement);
// Calls the underlying method on the IIterable to retrieve the last element of the IIterable object, typically is invoked by an IIterator object.
// Should return a pointer to the requested element on success, otherwise a null pointer is returned.
CSC_PVOID CSCMETHOD CSC_IIterableLastElement(_In_ CONST CSC_IIterable* CONST pThis);
// Calls the underlying method on the IIterable to retrieve the previous element of the IIterable object, typically is invoked by an IIterator object.
// The method needs the invoker to pass the current index aswell as a pointer to the current element of the iteration, so that the IIterable can find the previous element if it exists.
// Should return a pointer to the requested element on success, otherwise a null pointer is returned.
CSC_PVOID CSCMETHOD CSC_IIterablePreviousElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement);

// Calls the underlying method on the IIterable to retrieve the requested element of the IIterable object, typically is invoked by an IIterator object.
// If there is currently a valid iteration, the index and address of the current element can be passed so that the IIterable can take advantage of these inputs for accessing the requested element.
// If the optional inputs are not used, the pCurrentElement parameter must be a nullptr and the currentIndex parameter must be CSC_ITERATOR_INVALID_INDEX.
// Should return a pointer to the requested element on success, otherwise a null pointer is returned.
CSC_PVOID CSCMETHOD CSC_IIterableGetElementAt(_In_ CONST CSC_IIterable* CONST pThis, _In_ CONST CSC_SIZE_T index, _In_opt_ CONST CSC_SIZE_T currentIndex, _In_opt_ CONST CSC_PVOID pCurrentElement);
// Calls the underlying method on the IIterable to retrieve the element count of the IIterable object, typically is invoked by an IIterator object.
// On success the element count of the IIterable is returned, otherwise CSC_ITERATOR_INVALID_LENGTH is returned.
CSC_SIZE_T CSCMETHOD CSC_IIterableGetElementCount(_In_ CONST CSC_IIterable* CONST pThis);

#endif