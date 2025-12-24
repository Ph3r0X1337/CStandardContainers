#include "IAllocator.h"

CSC_STATUS CSCMETHOD CSC_IAllocatorInit(_Inout_ struct _CSC_IAllocator* CONST pThis)
{
	if (!pThis || !pThis->pIAllocatorVirtualTable || !pThis->pIAllocatorVirtualTable->pInitialize)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIAllocatorVirtualTable->pInitialize(pThis);
	}
}

CSC_STATUS CSCMETHOD CSC_IAllocatorCleanup(_Inout_ struct _CSC_IAllocator* CONST pThis)
{
	if (!pThis || !pThis->pIAllocatorVirtualTable || !pThis->pIAllocatorVirtualTable->pCleanup)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIAllocatorVirtualTable->pCleanup(pThis);
	}
}


CSC_PVOID CSCMETHOD CSC_IAllocatorAlloc(_In_ CONST struct _CSC_IAllocator* CONST pThis, _In_ CONST CSC_SIZE_T size)
{
	if (!size || !pThis || !pThis->pIAllocatorVirtualTable || !pThis->pIAllocatorVirtualTable->pAlloc)
	{
		return NULL;
	}
	else
	{
		return pThis->pIAllocatorVirtualTable->pAlloc(pThis, size);
	}
}

CSC_PVOID CSCMETHOD CSC_IAllocatorAllocZero(_In_ CONST struct _CSC_IAllocator* CONST pThis, _In_ CONST CSC_SIZE_T size)
{
	if (!size || !pThis || !pThis->pIAllocatorVirtualTable || !pThis->pIAllocatorVirtualTable->pAllocZero)
	{
		return NULL;
	}
	else
	{
		return pThis->pIAllocatorVirtualTable->pAllocZero(pThis, size);
	}
}


CSC_STATUS CSCMETHOD CSC_IAllocatorFree(_In_ CONST struct _CSC_IAllocator* CONST pThis, _In_ CONST CSC_PVOID pMemoryBlock)
{
	if (!pMemoryBlock || !pThis || !pThis->pIAllocatorVirtualTable || !pThis->pIAllocatorVirtualTable->pFree)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return pThis->pIAllocatorVirtualTable->pFree(pThis, pMemoryBlock);
	}
}


CSC_BOOLEAN CSCMETHOD CSC_IAllocatorIsUsable(_In_ CONST struct _CSC_IAllocator* CONST pThis)
{
	if (!pThis || !pThis->pIAllocatorVirtualTable || !pThis->pIAllocatorVirtualTable->pIsUsable)
	{
		return (CSC_BOOLEAN)FALSE;
	}
	else
	{
		return pThis->pIAllocatorVirtualTable->pIsUsable(pThis);
	}
}