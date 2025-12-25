#include "IIterable.h"

CSC_STATUS CSCMETHOD CSC_IIterableRegisterIterator(_Inout_ struct _CSC_IIterable* CONST pThis, _In_ CONST CSC_IIterator* CONST pIIterator)
{
	if (!pIIterator || !pThis || !pThis->pIIterableVirtualTable || !pThis->pIIterableVirtualTable->pRegisterIterator)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIIterableVirtualTable->pRegisterIterator(pThis, pIIterator);
	}
}

CSC_STATUS CSCMETHOD CSC_IIterableUnregisterIterator(_Inout_ struct _CSC_IIterable* CONST pThis)
{
	if (!pThis || !pThis->pIIterableVirtualTable || !pThis->pIIterableVirtualTable->pUnregisterIterator)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIIterableVirtualTable->pUnregisterIterator(pThis);
	}
}


CSC_PVOID CSCMETHOD CSC_IIterableFirstElement(_In_ CONST struct _CSC_IIterable* CONST pThis)
{
	if (!pThis || !pThis->pIIterableVirtualTable || !pThis->pIIterableVirtualTable->pFirstElement)
	{
		return NULL;
	}
	else
	{
		return pThis->pIIterableVirtualTable->pFirstElement(pThis);
	}
}

CSC_PVOID CSCMETHOD CSC_IIterableNextElement(_In_ CONST struct _CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement)
{
	if (!pThis || !pThis->pIIterableVirtualTable || !pThis->pIIterableVirtualTable->pNextElement)
	{
		return NULL;
	}
	else
	{
		return pThis->pIIterableVirtualTable->pNextElement(pThis, currentIndex, pCurrentElement);
	}
}

CSC_PVOID CSCMETHOD CSC_IIterableLastElement(_In_ CONST struct _CSC_IIterable* CONST pThis)
{
	if (!pThis || !pThis->pIIterableVirtualTable || !pThis->pIIterableVirtualTable->pLastElement)
	{
		return NULL;
	}
	else
	{
		return pThis->pIIterableVirtualTable->pLastElement(pThis);
	}
}

CSC_PVOID CSCMETHOD CSC_IIterablePreviousElement(_In_ CONST struct _CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement)
{
	if (!pThis || !pThis->pIIterableVirtualTable || !pThis->pIIterableVirtualTable->pPreviousElement)
	{
		return NULL;
	}
	else
	{
		return pThis->pIIterableVirtualTable->pPreviousElement(pThis, currentIndex, pCurrentElement);
	}
}


CSC_PVOID CSCMETHOD CSC_IIterableGetElementAt(_In_ CONST struct _CSC_IIterable* CONST pThis, _In_ CONST CSC_SIZE_T index, _In_opt_ CONST CSC_SIZE_T currentIndex, _In_opt_ CONST CSC_PVOID pCurrentElement)
{
	if (!pThis || !pThis->pIIterableVirtualTable || !pThis->pIIterableVirtualTable->pGetElementAt)
	{
		return NULL;
	}
	else
	{
		return pThis->pIIterableVirtualTable->pGetElementAt(pThis, index, currentIndex, pCurrentElement);
	}
}

CSC_SIZE_T CSCMETHOD CSC_IIterableGetElementCount(_In_ CONST struct _CSC_IIterable* CONST pThis)
{
	if (!pThis || !pThis->pIIterableVirtualTable || !pThis->pIIterableVirtualTable->pLastElement)
	{
		return CSC_ITERATOR_INVALID_LENGTH;
	}
	else
	{
		return pThis->pIIterableVirtualTable->pLastElement(pThis);
	}
}
