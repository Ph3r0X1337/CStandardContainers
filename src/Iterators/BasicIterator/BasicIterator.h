#ifndef CSC_BASIC_ITERATOR
#define CSC_BASIC_ITERATOR

#ifdef __cplusplus
extern "C" {
#endif

/*
Description:
The basic iterator implements the IIterator interface and can be bound to any object implementing the IIterable interface.
To bind the basic iterator to an iterable object, it must first be initialized.
Furthermore, the pointer to the IIterable interface of the object must be queried before binding.
Once initialized, passing the queried IIterable interface pointer to the CSC_BasicIteratorRegisterIterable method binds the iterator to the iterable.
Binding to another iterable will automatically unbind the old iterable and invalidate the previous iteration metadata.
Using the CSC_BasicIteratorUnregisterIterable method the same can be achieved without binding to a new object.
Once the basic iterator is bound to an iterable, it will be notified by the iterable, if the iterable is modified during an active iteration.
The iterable will either update the iteration to it's best effort, or invalidate it if updating the iteration is impossible.
This behaviour depends on the implementation of the iterable, hence check out it's documentation to make sure you iterate safely.
Using the other methods of the basic iterator, the user can query metadata of the iterable and the iterator or perform iteration.
Once done using an instance of the basic iterator, the CSC_BasicIteratorDestroy method should be used to perform proper cleanup.
Keep in mind that this will destroy the iterator object itself, making it unusable until the instance is initialized again.
In such a case the use of the CSC_BasicIteratorUnregisterIterable method should be preferred.
*/

/* Include the current configuration of the library containing various type definitions and other things. */
#include "../../Configuration/Configuration.h"
/* Include the IBaseInterface as the BasicIterator needs it to implement it's own IIterator interface. */
#include "../../Interfaces/IBaseInterface.h"
/* Include the IIterator interface as BasicIterator implements this interface. */
#include "../../Interfaces/IIterator.h"
/* Include the IIterable interface as the BasicIterator requires working with the IIterable interface. */
#include "../../Interfaces/IIterable.h"

/* Definition of BasicIterator type. */
typedef struct _CSC_BasicIterator
{
    CSC_IBaseInterface baseInterface;
    CSC_IIterator iteratorInterface;
    CSC_IIterable* pIIterable;
    CSC_SIZE_T elementSize;
    CSC_SIZE_T elementCount;
    CSC_SIZE_T currentIndex;
    CSC_PVOID pCurrentElement;
    CSC_BOOLEAN iterationValid;
} CSC_BasicIterator;

/* Initialization method of the basic iterator type. */
/* Sets up the internal data of a valid basic iterator object, the method will fail on already initialized objects. */
CSC_STATUS CSCMETHOD CSC_BasicIteratorInitialize(_When_(return == CSC_STATUS_SUCCESS, _Out_) CSC_BasicIterator* CONST pThis);
/* Object destructor of the basic iterator type. */
/* Invalidates the active iteration (if it exists), unregisters the iterator from a bound iterable (if it exists) and destructs the iterator instance. */
/* Destruction will fail when used on uninitialized or already destroyed objects. */
CSC_STATUS CSCMETHOD CSC_BasicIteratorDestroy(_Inout_ CSC_BasicIterator* CONST pThis);

/* Method used to bind to an object implementing the IIterable interface. */
/* If an iterable is already bound to the iterator instance, the method will invalidate an active iteration and unbind from the iterable, before binding to the new iterable. */
/* Whether binding to the iterable succeeds when another iterator is already bound to it, depends on the implementation of the iterable. */
/* However, you can assume that the iterable implementations provided by the CSC library will refuse such attempts, hence you'll need to make sure no iterator is bound before attempting to bind to the iterable. */
CSC_STATUS CSCMETHOD CSC_BasicIteratorRegisterIterable(_Inout_ CSC_BasicIterator* CONST pThis, _Inout_ CSC_IIterable* CONST pIIterable);
/* Method used to unbind from the iterable the iterator instance is currently bound to. */
/* Before the unbinding is attempted, an active iteration will be invalidated. */
/* The method will fail, if no iterable object is currently bound. */
CSC_STATUS CSCMETHOD CSC_BasicIteratorUnregisterIterable(_Inout_ CSC_BasicIterator* CONST pThis);

/* Method used to move the iterator to the first element of the iterable bound to the iterator. */
/* The method can be used to initiate an iteration, it will return the address of the first element of the iterable. */
/* If the iterator isn't bound to any iterable, the method will not succeed. */
/* On failure, the method returns an null pointer. */
CSC_PVOID CSCMETHOD CSC_BasicIteratorFirstElement(_Inout_ CSC_BasicIterator* CONST pThis);
/* Method used to move the iterator to the next element of the iterable bound to the iterator. */
/* The method requires an already active iteration to succeed, it will return the address of the next element of the iterable. */
/* If the iterator isn't bound to any iterable or if the index goes out of bounds, the method fails and the iteration gets invalidated. */
/* On failure, the method returns an null pointer. */
CSC_PVOID CSCMETHOD CSC_BasicIteratorNextElement(_Inout_ CSC_BasicIterator* CONST pThis);
/* Method used to move the iterator to the last element of the iterable bound to the iterator. */
/* The method can be used to initiate an iteration, it will return the address of the last element of the iterable. */
/* If the iterator isn't bound to any iterable, the method will not succeed. */
/* On failure, the method returns an null pointer. */
CSC_PVOID CSCMETHOD CSC_BasicIteratorLastElement(_Inout_ CSC_BasicIterator* CONST pThis);
/* Method used to move the iterator to the previous element of the iterable bound to the iterator. */
/* The method requires an already active iteration to succeed, it will return the address of the previous element of the iterable. */
/* If the iterator isn't bound to any iterable or if the index goes out of bounds, the method fails and the iteration gets invalidated. */
/* On failure, the method returns an null pointer. */
CSC_PVOID CSCMETHOD CSC_BasicIteratorPreviousElement(_Inout_ CSC_BasicIterator* CONST pThis);
/* Method used to move the iterator to the element at the given index of the iterable bound to the iterator. */
/* The method can be used to initiate an iteration, it will return the address of the element at the given index of the iterable. */
/* If the iterator isn't bound to any iterable or if the index is out of bounds, the method fails and an active iteration gets invalidated. */
/* On failure, the method returns an null pointer. */
CSC_PVOID CSCMETHOD CSC_BasicIteratorMoveToIndex(_Inout_ CSC_BasicIterator* CONST pThis, _In_ CONST CSC_SIZE_T index);

/* Method used to manually invalidate an active iteration on the bound iterable. */
/* The method only fails if the passed iterator instance is invalid, the method will not fail if no iterable is bound or no iteration is active. */
CSC_STATUS CSCMETHOD CSC_BasicIteratorInvalidateIteration(_Inout_ CSC_BasicIterator* CONST pThis);

/* Method used to query the element size in bytes of the elements accessed through the bound iterable. */
/* On failure the method returns zero, such as when the iterator instance is invalid or if no iterable is bound. */
CSC_SIZE_T CSCMETHOD CSC_BasicIteratorGetElementSize(_In_ CONST CSC_BasicIterator* CONST pThis);
/* Method used to query the element count of the elements accessed through the bound iterable. */
/* On failure the method returns CSC_ITERATOR_INVALID_LENGTH, such as when the iterator instance is invalid or if no iterable is bound. */
CSC_SIZE_T CSCMETHOD CSC_BasicIteratorGetElementCount(_In_ CONST CSC_BasicIterator* CONST pThis);
/* Method used to query the current index of the active iteration on the bound iterable. */
/* On failure the method returns CSC_ITERATOR_INVALID_INDEX, such as when the iterator instance is invalid or if no iterable is bound or if there is no active iteration. */
CSC_SIZE_T CSCMETHOD CSC_BasicIteratorGetCurrentIndex(_In_ CONST CSC_BasicIterator* CONST pThis);
/* Method used to query the address of the element at the current index of the active iteration on the bound iterable. */
/* On failure the method returns a null pointer, such as when the iterator instance is invalid or if no iterable is bound or if there is no active iteration. */
CSC_PVOID CSCMETHOD CSC_BasicIteratorGetCurrentElement(_In_ CONST CSC_BasicIterator* CONST pThis);
/* Method used to query the address of the element at the given index of the bound iterable. */
/* On failure the method returns a null pointer, such as when the iterator instance is invalid or if no iterable is bound or if the index is out of bounds. */
CSC_PVOID CSCMETHOD CSC_BasicIteratorGetElementAt(_In_ CONST CSC_BasicIterator* CONST pThis, _In_ CONST CSC_SIZE_T index);

/* Method used to query the IBaseInterface pointer of the basic iterator instance, which can be used to query the other interfaces implemented by the object. */
CSC_IBaseInterface* CSCMETHOD CSC_BasicIteratorGetIBaseInterface(_In_ CONST CSC_BasicIterator* CONST pThis);
/* Method used to query the IIterator interface pointer of the basic iterator instance. */
CSC_IIterator* CSCMETHOD CSC_BasicIteratorGetIIterator(_In_ CONST CSC_BasicIterator* CONST pThis);

/* Method used to check whether an instance of the basic iterator is in a valid state. */
CSC_STATUS CSCMETHOD CSC_BasicIteratorIsValid(_In_ CONST CSC_BasicIterator* CONST pThis);
/* Method used to check whether an iterable is currently bound to the basic iterator instance. */
CSC_BOOLEAN CSCMETHOD CSC_BasicIteratorIsRegistered(_In_ CONST CSC_BasicIterator* CONST pThis);
/* Method used to check whether the iterator holds the metadata of an active iteration on the bound iterable. */
CSC_BOOLEAN CSCMETHOD CSC_BasicIteratorIsIterationValid(_In_ CONST CSC_BasicIterator* CONST pThis);

#ifdef __cplusplus
};
#endif

#endif
