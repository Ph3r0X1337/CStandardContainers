#include "LinkedList.h"


CSC_STATUS CSCMETHOD CSC_LinkedListInitialize(_Out_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_BOOLEAN circular, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListInitializeWithSize(_Out_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_BOOLEAN circular, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListDestroy(_Inout_ CSC_LinkedList* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListErase(_Inout_ CSC_LinkedList* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListZeroMemory(_Out_ CSC_LinkedList* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListInsertElement(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_opt_ CONST CSC_PCVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListInsertRange(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListInsertListCopy(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_LinkedList* CONST pOther)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListInsertListMove(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _Inout_ CSC_LinkedList* CONST pOther)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListRemoveElement(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListRemoveRange(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListPushElement(_Inout_ CSC_LinkedList* CONST pThis, _In_opt_ CONST CSC_PCVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListPopElement(_Inout_ CSC_LinkedList* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListAppendCopy(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_LinkedList* CONST pOther)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListAppendMove(_Inout_ CSC_LinkedList* CONST pThis, _Inout_ CSC_LinkedList* CONST pOther)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListRotateForward(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T rotationCount)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListRotateForwardByOne(_Inout_ CSC_LinkedList* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListRotateBackward(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T rotationCount)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListRotateBackwardByOne(_Inout_ CSC_LinkedList* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListRotateToIndex(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T newHeadIndex)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListOpen(_Inout_ CSC_LinkedList* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListClose(_Inout_ CSC_LinkedList* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}


CSC_PVOID CSCMETHOD CSC_LinkedListAccessElement(_In_ CONST CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T index)
{
	return NULL;
}


CSC_STATUS CSCMETHOD CSC_LinkedListReverse(_Inout_ CSC_LinkedList* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}


CSC_PVOID CSCMETHOD CSC_LinkedListFront(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return NULL;
}

CSC_PVOID CSCMETHOD CSC_LinkedListBack(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return NULL;
}


CSC_STATUS CSCMETHOD CSC_LinkedListIsEmpty(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListIsValid(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}


CSC_SIZE_T CSCMETHOD CSC_LinkedListGetSize(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (CSC_SIZE_T)0;
}

CSC_SIZE_T CSCMETHOD CSC_LinkedListGetMaxElements(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (CSC_SIZE_T)0;
}

CSC_SIZE_T CSCMETHOD CSC_LinkedListGetElementSize(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (CSC_SIZE_T)0;
}

CSC_STATUS CSCMETHOD CSC_LinkedListGetCircular(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}

CSC_IBaseInterface* CSCMETHOD CSC_LinkedListGetIBaseInterface(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (CSC_IBaseInterface*)NULL;
}

CSC_IContainer* CSCMETHOD CSC_LinkedListGetIContainer(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (CSC_IContainer*)NULL;
}

CSC_IIterable* CSCMETHOD CSC_LinkedListGetIIterable(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (CSC_IIterable*)NULL;
}

CSC_IAllocator* CSCMETHOD CSC_LinkedListGetIAllocator(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (CSC_IAllocator*)NULL;
}

CSC_IContainerVirtualTable* CSCMETHOD CSC_LinkedListGetNestedContainerVTable(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (CSC_IContainerVirtualTable*)NULL;
}
