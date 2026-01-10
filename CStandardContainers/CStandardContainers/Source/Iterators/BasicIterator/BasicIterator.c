#include "BasicIterator.h"
#include <stddef.h>

typedef struct _CSC_BasicIteratorVirtualTable
{
	CSC_IBaseInterfaceVirtualTable baseInterfaceVTable;
	CSC_IIteratorVirtualTable iteratorInterfaceVTable;
} CSC_BasicIteratorVirtualTable;

static CSC_PCVOID CSCMETHOD CSC_BasicIteratorIBaseInterfaceGetInterface(_In_ CONST CSC_IBaseInterface* CONST pThis, _In_ CONST EBaseInterfaceType interfaceType);

static CSC_STATUS CSCMETHOD CSC_BasicIteratorIIteratorInvalidateIteration(_Inout_ CSC_IIterator* CONST pThis);
static CSC_STATUS CSCMETHOD CSC_BasicIteratorIIteratorUpdateIteration(_Inout_ CSC_IIterator* CONST pThis);
static CSC_STATUS CSCMETHOD CSC_BasicIteratorIIteratorOnInsertion(_Inout_ CSC_IIterator* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T newSize);
static CSC_STATUS CSCMETHOD CSC_BasicIteratorIIteratorOnRemoval(_Inout_ CSC_IIterator* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T newSize);
static CSC_STATUS CSCMETHOD CSC_BasicIteratorIIteratorOnDestruction(_Inout_ CSC_IIterator* CONST pThis);

static CONST CSC_BasicIteratorVirtualTable g_basicIteratorVirtualTable =
{
	{
		CSC_BasicIteratorIBaseInterfaceGetInterface
	},
	{
		CSC_BasicIteratorIIteratorInvalidateIteration,
		CSC_BasicIteratorIIteratorUpdateIteration,
		CSC_BasicIteratorIIteratorOnInsertion,
		CSC_BasicIteratorIIteratorOnRemoval,
		CSC_BasicIteratorIIteratorOnDestruction
	}
};

CSC_STATUS CSCMETHOD CSC_BasicIteratorZeroMemory(_Out_ CSC_BasicIterator* CONST pThis)
{
	if (!pThis)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	pThis->baseInterface.pIBaseInterfaceVirtualTable = (CSC_IBaseInterfaceVirtualTable*)NULL;
	pThis->iteratorInterface.pIIteratorVirtualTable = (CSC_IIteratorVirtualTable*)NULL;

	pThis->iterationValid = (CSC_BOOLEAN)FALSE;

	pThis->elementCount = (CSC_SIZE_T)0;
	pThis->elementSize = (CSC_SIZE_T)0;
	pThis->currentIndex = (CSC_SIZE_T)0;

	pThis->pCurrentElement = NULL;

	pThis->pIIterable = (CSC_IIterable*)NULL;

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_BasicIteratorInitialize(_Out_ CSC_BasicIterator* CONST pThis)
{
	CSC_STATUS status = CSC_BasicIteratorZeroMemory(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	pThis->baseInterface.pIBaseInterfaceVirtualTable = (CSC_IBaseInterfaceVirtualTable*)&g_basicIteratorVirtualTable.baseInterfaceVTable;
	pThis->iteratorInterface.pIIteratorVirtualTable = (CSC_IIteratorVirtualTable*)&g_basicIteratorVirtualTable.iteratorInterfaceVTable;

	pThis->currentIndex = CSC_ITERATOR_INVALID_INDEX;
	pThis->elementCount = CSC_ITERATOR_INVALID_LENGTH;
	pThis->elementSize = (CSC_SIZE_T)0;
	pThis->pCurrentElement = NULL;
	pThis->iterationValid = (CSC_BOOLEAN)FALSE;

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_BasicIteratorDestroy(_Inout_ CSC_BasicIterator* CONST pThis)
{
	CSC_STATUS status = CSC_BasicIteratorInvalidateIteration(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (pThis->pIIterable)
	{
		status = CSC_IIterableUnregisterIterator(pThis->pIIterable, &pThis->iteratorInterface);

		if (status != CSC_STATUS_SUCCESS)
		{
			return status;
		}
	}

	return CSC_BasicIteratorZeroMemory(pThis);
}


CSC_STATUS CSCMETHOD CSC_BasicIteratorRegisterIterable(_Inout_ CSC_BasicIterator* CONST pThis, _Inout_ CSC_IIterable* CONST pIIterable)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_BasicIteratorUnregisterIterable(_Inout_ CSC_BasicIterator* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}


CSC_PVOID CSCMETHOD CSC_BasicIteratorFirstElement(_Inout_ CSC_BasicIterator* CONST pThis)
{
	return NULL;
}

CSC_PVOID CSCMETHOD CSC_BasicIteratorNextElement(_Inout_ CSC_BasicIterator* CONST pThis)
{
	return NULL;
}

CSC_PVOID CSCMETHOD CSC_BasicIteratorLastElement(_Inout_ CSC_BasicIterator* CONST pThis)
{
	return NULL;
}

CSC_PVOID CSCMETHOD CSC_BasicIteratorPreviousElement(_Inout_ CSC_BasicIterator* CONST pThis)
{
	return NULL;
}

CSC_PVOID CSCMETHOD CSC_BasicIteratorMoveToIndex(_Inout_ CSC_BasicIterator* CONST pThis)
{
	return NULL;
}


CSC_STATUS CSCMETHOD CSC_BasicIteratorInvalidateIteration(_Inout_ CSC_BasicIterator* CONST pThis)
{
	if (!pThis || pThis->baseInterface.pIBaseInterfaceVirtualTable != (CSC_IBaseInterfaceVirtualTable*)&g_basicIteratorVirtualTable.baseInterfaceVTable || pThis->iteratorInterface.pIIteratorVirtualTable != (CSC_IIteratorVirtualTable*)&g_basicIteratorVirtualTable.iteratorInterfaceVTable)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	pThis->currentIndex = CSC_ITERATOR_INVALID_INDEX;
	pThis->elementCount = CSC_ITERATOR_INVALID_LENGTH;
	pThis->elementSize = (CSC_SIZE_T)0;
	pThis->pCurrentElement = NULL;
	pThis->iterationValid = (CSC_BOOLEAN)FALSE;

	return CSC_STATUS_SUCCESS;
}


CSC_SIZE_T CSCMETHOD CSC_BasicIteratorGetElementSize(_In_ CONST CSC_BasicIterator* CONST pThis)
{
	return (CSC_SIZE_T)0;
}

CSC_SIZE_T CSCMETHOD CSC_BasicIteratorGetElementCount(_In_ CONST CSC_BasicIterator* CONST pThis)
{
	return (CSC_SIZE_T)0;
}

CSC_SIZE_T CSCMETHOD CSC_BasicIteratorGetCurrentIndex(_In_ CONST CSC_BasicIterator* CONST pThis)
{
	return (CSC_SIZE_T)0;
}

CSC_PVOID CSCMETHOD CSC_BasicIteratorGetCurrentElement(_In_ CONST CSC_BasicIterator* CONST pThis)
{
	return NULL;
}

CSC_PVOID CSCMETHOD CSC_BasicIteratorGetElementAt(_In_ CONST CSC_BasicIterator* CONST pThis)
{
	return NULL;
}


CSC_IBaseInterface* CSCMETHOD CSC_BasicIteratorGetIBaseInterface(_In_ CONST CSC_BasicIterator* CONST pThis)
{
	return (CSC_IBaseInterface*)((CSC_BasicIteratorIsValid(pThis) != CSC_STATUS_SUCCESS) ? NULL : &pThis->baseInterface);
}

CSC_IIterator* CSCMETHOD CSC_BasicIteratorGetIIterator(_In_ CONST CSC_BasicIterator* CONST pThis)
{
	return (CSC_IIterator*)((CSC_BasicIteratorIsValid(pThis) != CSC_STATUS_SUCCESS) ? NULL : &pThis->iteratorInterface);
}


CSC_STATUS CSCMETHOD CSC_BasicIteratorIsValid(_In_ CONST CSC_BasicIterator* CONST pThis)
{
	return (!pThis || pThis->baseInterface.pIBaseInterfaceVirtualTable != (CSC_IBaseInterfaceVirtualTable*)&g_basicIteratorVirtualTable.baseInterfaceVTable || pThis->iteratorInterface.pIIteratorVirtualTable != (CSC_IIteratorVirtualTable*)&g_basicIteratorVirtualTable.iteratorInterfaceVTable || pThis->iterationValid && (!pThis->pIIterable || pThis->currentIndex == CSC_ITERATOR_INVALID_INDEX || pThis->elementCount == CSC_ITERATOR_INVALID_LENGTH || !pThis->pCurrentElement || !pThis->elementSize)) ? CSC_STATUS_INVALID_PARAMETER : CSC_STATUS_SUCCESS;
}

CSC_BOOLEAN CSCMETHOD CSC_BasicIteratorIsRegistered(_In_ CONST CSC_BasicIterator* CONST pThis)
{
	return (CSC_BOOLEAN)((CSC_BasicIteratorIsValid(pThis) != CSC_STATUS_SUCCESS || !pThis->pIIterable) ? FALSE : TRUE);
}

CSC_BOOLEAN CSCMETHOD CSC_BasicIteratorIsIterationValid(_In_ CONST CSC_BasicIterator* CONST pThis)
{
	return (CSC_BasicIteratorIsValid(pThis) != CSC_STATUS_SUCCESS || !pThis->pIIterable) ? (CSC_BOOLEAN)FALSE : pThis->iterationValid;
}


static CSC_PCVOID CSCMETHOD CSC_BasicIteratorIBaseInterfaceGetInterface(_In_ CONST CSC_IBaseInterface* CONST pThis, _In_ CONST EBaseInterfaceType interfaceType)
{
	CONST CSC_BasicIterator* CONST pBasicIterator = (CONST CSC_BasicIterator* CONST)pThis;

	if (!pBasicIterator || CSC_BasicIteratorIsValid(pBasicIterator) != CSC_STATUS_SUCCESS)
	{
		return (CSC_PCVOID)NULL;
	}

	switch (interfaceType)
	{
	case csc_bit_IIterator:

		return (CSC_PCVOID)&pBasicIterator->iteratorInterface;

	default:

		return (CSC_PCVOID)NULL;
	}

	return (CSC_PCVOID)NULL;
}


static CSC_BasicIterator* CSCMETHOD CSC_BasicIteratorIIteratorGetObjectPointer(_In_ CONST  CSC_IIterator* CONST pThis)
{
	if (!pThis || pThis->pIIteratorVirtualTable != &g_basicIteratorVirtualTable.iteratorInterfaceVTable)
	{
		return (CSC_BasicIterator*)NULL;
	}
	else
	{
		return (CSC_BasicIterator*)((CONST CSC_BYTE* CONST)pThis - (CSC_SIZE_T)offsetof(CSC_BasicIterator, iteratorInterface));
	}
}

static CSC_STATUS CSCMETHOD CSC_BasicIteratorIIteratorInvalidateIteration(_Inout_ CSC_IIterator* CONST pThis)
{
	CSC_BasicIterator* pBasicIterator = CSC_BasicIteratorIIteratorGetObjectPointer(pThis);

	if (!pBasicIterator)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_BasicIteratorInvalidateIteration(pBasicIterator);
	}
}

static CSC_STATUS CSCMETHOD CSC_BasicIteratorIIteratorUpdateIteration(_Inout_ CSC_IIterator* CONST pThis)
{
	CSC_STATUS status;
	CSC_BasicIterator* pBasicIterator = CSC_BasicIteratorIIteratorGetObjectPointer(pThis);

	if (!pBasicIterator)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		status = CSC_BasicIteratorIsValid(pBasicIterator);

		if (status != CSC_STATUS_SUCCESS)
		{
			return status;
		}

		if (!pBasicIterator->pIIterable)
		{
			return CSC_STATUS_INVALID_PARAMETER;
		}
		
		if (!pBasicIterator->iterationValid)
		{
			return CSC_STATUS_SUCCESS;
		}

		pBasicIterator->elementCount = CSC_IIterableGetElementCount(pBasicIterator->pIIterable);

		if (pBasicIterator->elementCount == CSC_ITERATOR_INVALID_LENGTH || pBasicIterator->currentIndex >= pBasicIterator->elementCount)
		{
			CSC_BasicIteratorInvalidateIteration(pBasicIterator);
		}
		else
		{
			pBasicIterator->pCurrentElement = CSC_IIterableGetElementAt(pBasicIterator->pIIterable, pBasicIterator->currentIndex, CSC_ITERATOR_INVALID_INDEX, NULL);

			if (!pBasicIterator->pCurrentElement)
			{
				CSC_BasicIteratorInvalidateIteration(pBasicIterator);
			}
		}

		return CSC_STATUS_SUCCESS;
	}
}

static CSC_STATUS CSCMETHOD CSC_BasicIteratorIIteratorOnInsertion(_Inout_ CSC_IIterator* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T newSize)
{
	CSC_STATUS status;
	CSC_BasicIterator* pBasicIterator = CSC_BasicIteratorIIteratorGetObjectPointer(pThis);

	if (!pBasicIterator)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		status = CSC_BasicIteratorIsValid(pBasicIterator);

		if (status != CSC_STATUS_SUCCESS)
		{
			return status;
		}

		if (!pBasicIterator->pIIterable)
		{
			return CSC_STATUS_INVALID_PARAMETER;
		}

		if (!pBasicIterator->iterationValid)
		{
			return CSC_STATUS_SUCCESS;
		}

		pBasicIterator->elementCount = newSize;

		if (numOfElements && pBasicIterator->currentIndex >= insertIndex)
		{
			pBasicIterator->currentIndex += numOfElements;
		}

		if (pBasicIterator->elementCount == CSC_ITERATOR_INVALID_LENGTH || pBasicIterator->currentIndex >= pBasicIterator->elementCount)
		{
			CSC_BasicIteratorInvalidateIteration(pBasicIterator);
		}
		else
		{
			pBasicIterator->pCurrentElement = CSC_IIterableGetElementAt(pBasicIterator->pIIterable, pBasicIterator->currentIndex, CSC_ITERATOR_INVALID_INDEX, NULL);

			if (!pBasicIterator->pCurrentElement)
			{
				CSC_BasicIteratorInvalidateIteration(pBasicIterator);
			}
		}

		return CSC_STATUS_SUCCESS;
	}
}

static CSC_STATUS CSCMETHOD CSC_BasicIteratorIIteratorOnRemoval(_Inout_ CSC_IIterator* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T newSize)
{
	CSC_STATUS status;
	CSC_BasicIterator* pBasicIterator = CSC_BasicIteratorIIteratorGetObjectPointer(pThis);

	if (!pBasicIterator)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		status = CSC_BasicIteratorIsValid(pBasicIterator);

		if (status != CSC_STATUS_SUCCESS)
		{
			return status;
		}

		if (!pBasicIterator->pIIterable)
		{
			return CSC_STATUS_INVALID_PARAMETER;
		}

		if (!pBasicIterator->iterationValid)
		{
			return CSC_STATUS_SUCCESS;
		}

		pBasicIterator->elementCount = newSize;

		if (numOfElements && pBasicIterator->currentIndex >= removeIndex)
		{
			if (pBasicIterator->currentIndex < removeIndex + numOfElements)
			{
				CSC_BasicIteratorInvalidateIteration(pBasicIterator);
				return CSC_STATUS_SUCCESS;
			}
			else
			{
				pBasicIterator->currentIndex -= numOfElements;
			}
		}

		if (pBasicIterator->elementCount == CSC_ITERATOR_INVALID_LENGTH || pBasicIterator->currentIndex >= pBasicIterator->elementCount)
		{
			CSC_BasicIteratorInvalidateIteration(pBasicIterator);
		}
		else
		{
			pBasicIterator->pCurrentElement = CSC_IIterableGetElementAt(pBasicIterator->pIIterable, pBasicIterator->currentIndex, CSC_ITERATOR_INVALID_INDEX, NULL);

			if (!pBasicIterator->pCurrentElement)
			{
				CSC_BasicIteratorInvalidateIteration(pBasicIterator);
			}
		}

		return CSC_STATUS_SUCCESS;
	}
}

static CSC_STATUS CSCMETHOD CSC_BasicIteratorIIteratorOnDestruction(_Inout_ CSC_IIterator* CONST pThis)
{
	CSC_STATUS status;
	CSC_BasicIterator* pBasicIterator = CSC_BasicIteratorIIteratorGetObjectPointer(pThis);

	if (!pBasicIterator)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		status = CSC_BasicIteratorIsValid(pBasicIterator);

		if (status != CSC_STATUS_SUCCESS)
		{
			return status;
		}

		if (!pBasicIterator->pIIterable)
		{
			return CSC_STATUS_INVALID_PARAMETER;
		}

		CSC_BasicIteratorInvalidateIteration(pBasicIterator);
		pBasicIterator->pIIterable = (CSC_IIterable*)NULL;

		return CSC_STATUS_SUCCESS;
	}
}