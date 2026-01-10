#ifndef CSC_BASIC_ITERATOR
#define CSC_BASIC_ITERATOR

#ifdef __cplusplus
extern "C" {
#endif

/*
Description:

*/

// Include the current configuration of the library containing various type definitions and other things.
#include "../../Configuration/Configuration.h"
// Include the IBaseInterface as the BasicIterator needs it to implement it's own IIterator interface. 
#include "../../Interfaces/IBaseInterface.h"
// Include the IIterator interface as BasicIterator implements this interface.
#include "../../Interfaces/IIterator.h"
// Include the IIterable interface as the BasicIterator requires working with the IIterable interface.
#include "../../Interfaces/IIterable.h"

// Definition of BasicIterator type.
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

CSC_STATUS CSCMETHOD CSC_BasicIteratorZeroMemory(_Out_ CSC_BasicIterator* CONST pThis);
CSC_STATUS CSCMETHOD CSC_BasicIteratorInitialize(_Out_ CSC_BasicIterator* CONST pThis);
CSC_STATUS CSCMETHOD CSC_BasicIteratorDestroy(_Inout_ CSC_BasicIterator* CONST pThis);

CSC_STATUS CSCMETHOD CSC_BasicIteratorRegisterIterable(_Inout_ CSC_BasicIterator* CONST pThis, _Inout_ CSC_IIterable* CONST pIIterable);
CSC_STATUS CSCMETHOD CSC_BasicIteratorUnregisterIterable(_Inout_ CSC_BasicIterator* CONST pThis);

CSC_PVOID CSCMETHOD CSC_BasicIteratorFirstElement(_Inout_ CSC_BasicIterator* CONST pThis);
CSC_PVOID CSCMETHOD CSC_BasicIteratorNextElement(_Inout_ CSC_BasicIterator* CONST pThis);
CSC_PVOID CSCMETHOD CSC_BasicIteratorLastElement(_Inout_ CSC_BasicIterator* CONST pThis);
CSC_PVOID CSCMETHOD CSC_BasicIteratorPreviousElement(_Inout_ CSC_BasicIterator* CONST pThis);
CSC_PVOID CSCMETHOD CSC_BasicIteratorMoveToIndex(_Inout_ CSC_BasicIterator* CONST pThis, _In_ CONST CSC_SIZE_T index);

CSC_STATUS CSCMETHOD CSC_BasicIteratorInvalidateIteration(_Inout_ CSC_BasicIterator* CONST pThis);

CSC_SIZE_T CSCMETHOD CSC_BasicIteratorGetElementSize(_In_ CONST CSC_BasicIterator* CONST pThis);
CSC_SIZE_T CSCMETHOD CSC_BasicIteratorGetElementCount(_In_ CONST CSC_BasicIterator* CONST pThis);
CSC_SIZE_T CSCMETHOD CSC_BasicIteratorGetCurrentIndex(_In_ CONST CSC_BasicIterator* CONST pThis);
CSC_PVOID CSCMETHOD CSC_BasicIteratorGetCurrentElement(_In_ CONST CSC_BasicIterator* CONST pThis);
CSC_PVOID CSCMETHOD CSC_BasicIteratorGetElementAt(_In_ CONST CSC_BasicIterator* CONST pThis, _In_ CONST CSC_SIZE_T index);

CSC_IBaseInterface* CSCMETHOD CSC_BasicIteratorGetIBaseInterface(_In_ CONST CSC_BasicIterator* CONST pThis);
CSC_IIterator* CSCMETHOD CSC_BasicIteratorGetIIterator(_In_ CONST CSC_BasicIterator* CONST pThis);

CSC_STATUS CSCMETHOD CSC_BasicIteratorIsValid(_In_ CONST CSC_BasicIterator* CONST pThis);
CSC_BOOLEAN CSCMETHOD CSC_BasicIteratorIsRegistered(_In_ CONST CSC_BasicIterator* CONST pThis);
CSC_BOOLEAN CSCMETHOD CSC_BasicIteratorIsIterationValid(_In_ CONST CSC_BasicIterator* CONST pThis);

#ifdef __cplusplus
};
#endif

#endif
