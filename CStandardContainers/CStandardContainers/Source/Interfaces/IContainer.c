#include "IContainer.h"

CSC_STATUS CSCMETHOD CSC_IContainerInitialize(_Out_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CSC_IAllocator* CONST pIAllocator)
{
	if (!elementSize || !pIAllocator || !pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pInitialize)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pInitialize(pThis, elementSize, pIAllocator);
	}
}

CSC_STATUS CSCMETHOD CSC_IContainerErase(_Inout_ CSC_IContainer* CONST pThis)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pErase)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pErase(pThis);
	}
}

CSC_STATUS CSCMETHOD CSC_IContainerDestroy(_Inout_ CSC_IContainer* CONST pThis)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pDestroy)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pDestroy(pThis);
	}
}


CSC_STATUS CSCMETHOD CSC_IContainerCopy(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST struct _CSC_IContainer* CONST pOther)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pCopy || !pOther || pOther->pIContainerVirtualTable != pThis->pIContainerVirtualTable)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pCopy(pThis, pOther);
	}
}

CSC_STATUS CSCMETHOD CSC_IContainerMove(_Inout_ CSC_IContainer* CONST pThis, _Inout_ struct _CSC_IContainer* CONST pOther)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pMove || !pOther || pOther->pIContainerVirtualTable != pThis->pIContainerVirtualTable)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pMove(pThis, pOther);
	}
}


CSC_STATUS CSCMETHOD CSC_IContainerInsertRange(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements)
{
	if (!numOfElements || !pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pInsertRange)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pInsertRange(pThis, insertIndex, numOfElements, pElements);
	}
}

CSC_STATUS CSCMETHOD CSC_IContainerRemoveRange(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements)
{
	if (!numOfElements || !pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pRemoveRange)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pRemoveRange(pThis, removeIndex, numOfElements);
	}
}

CSC_STATUS CSCMETHOD CSC_IContainerSwapValues(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T secondIndex)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pSwapValues)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pSwapValues(pThis, firstIndex, secondIndex);
	}
}


CSC_PVOID CSCMETHOD CSC_IContainerAccessElement(_In_ CONST CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T index)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pAccessElement)
	{
		return NULL;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pAccessElement(pThis, index);
	}
}


CSC_STATUS CSCMETHOD CSC_IContainerIsValid(_In_ CONST CSC_IContainer* CONST pThis)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pIsValid)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pIsValid(pThis);
	}
}

CSC_STATUS CSCMETHOD CSC_IContainerIsEmpty(_In_ CONST CSC_IContainer* CONST pThis)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pIsEmpty)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pIsEmpty(pThis);
	}
}

CSC_STATUS CSCMETHOD CSC_IContainerIsElementContainer(_In_ CONST CSC_IContainer* CONST pThis)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pIsElementContainer)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pIsElementContainer(pThis);
	}
}


CSC_SIZE_T CSCMETHOD CSC_IContainerGetSize(_In_ CONST CSC_IContainer* CONST pThis)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pGetSize)
	{
		return CSC_CONTAINER_INVALID_LENGTH;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pGetSize(pThis);
	}
}

CSC_SIZE_T CSCMETHOD CSC_IContainerGetElementSize(_In_ CONST CSC_IContainer* CONST pThis)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pGetElementSize)
	{
		return (CSC_SIZE_T)0;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pGetElementSize(pThis);
	}
}

CSC_SIZE_T CSCMETHOD CSC_IContainerGetMaxElements(_In_ CONST CSC_IContainer* CONST pThis)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pGetMaxElements)
	{
		return CSC_CONTAINER_INVALID_LENGTH;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pGetMaxElements(pThis);
	}
}

CSC_IAllocator* CSCMETHOD CSC_IContainerGetIAllocator(_In_ CONST CSC_IContainer* CONST pThis)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pGetIAllocator)
	{
		return (CSC_IAllocator*)NULL;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pGetIAllocator(pThis);
	}
}

CSC_IContainerVirtualTable* CSCMETHOD CSC_IContainerGetNestedContainerVTable(_In_ CONST struct _CSC_IContainer* CONST pThis)
{
	if (!pThis || !pThis->pIContainerVirtualTable || !pThis->pIContainerVirtualTable->pGetNestedContainerVTable)
	{
		return (CSC_IContainerVirtualTable*)NULL;
	}
	else
	{
		return pThis->pIContainerVirtualTable->pGetNestedContainerVTable(pThis);
	}
}
