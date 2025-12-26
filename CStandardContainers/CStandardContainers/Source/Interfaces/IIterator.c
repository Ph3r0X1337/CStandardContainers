#include "IIterator.h"

CSC_STATUS CSCMETHOD CSC_IIteratorInvalidateIteration(_Inout_ CSC_IIterator* CONST pThis)
{
	if (!pThis || !pThis->pIIteratorVirtualTable || !pThis->pIIteratorVirtualTable->pInvalidateIteration)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIIteratorVirtualTable->pInvalidateIteration(pThis);
	}
}

CSC_STATUS CSCMETHOD CSC_IIteratorUpdateIteration(_Inout_ CSC_IIterator* CONST pThis)
{
	if (!pThis || !pThis->pIIteratorVirtualTable || !pThis->pIIteratorVirtualTable->pUpdateIteration)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIIteratorVirtualTable->pUpdateIteration(pThis);
	}
}


CSC_STATUS CSCMETHOD CSC_IIteratorOnInsertion(_Inout_ CSC_IIterator* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T newSize)
{
	if (!numOfElements || insertIndex + numOfElements > newSize || !pThis || !pThis->pIIteratorVirtualTable || !pThis->pIIteratorVirtualTable->pOnInsertion)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIIteratorVirtualTable->pOnInsertion(pThis, insertIndex, numOfElements, newSize);
	}
}

CSC_STATUS CSCMETHOD CSC_IIteratorOnRemoval(_Inout_ CSC_IIterator* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T newSize)
{
	if (!numOfElements || removeIndex > newSize || !pThis || !pThis->pIIteratorVirtualTable || !pThis->pIIteratorVirtualTable->pOnRemoval)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIIteratorVirtualTable->pOnRemoval(pThis, removeIndex, numOfElements, newSize);
	}
}

CSC_STATUS CSCMETHOD CSC_IIteratorOnDestruction(_Inout_ CSC_IIterator* CONST pThis)
{
	if (!pThis || !pThis->pIIteratorVirtualTable || !pThis->pIIteratorVirtualTable->pOnDestruction)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIIteratorVirtualTable->pOnDestruction(pThis);
	}
}
