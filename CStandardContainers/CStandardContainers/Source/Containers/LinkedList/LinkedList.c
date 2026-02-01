#include "LinkedList.h"
#include "../../Interfaces/IBaseInterface.h"
#include "../../Utils/MemoryUtils.h"
#include <stddef.h>

/* Definition of container limits and behaviour based on the data bus address width. */
/* Architectures with an address width of 32 bits or more share the same restrictions in maximum size. */
#if CSC_ADDRESS_BUS_WIDTH_DATA < CSC_ADDRESS_BUS_WIDTH_DATA_32BIT
#define CSC_LINKED_LIST_MAXIMUM_SPACE (CSC_SIZE_T)0x4000
#define CSC_LINKED_LIST_MAXIMUM_ELEMENT_SIZE (CSC_SIZE_T)0x1000
#else
#define CSC_LINKED_LIST_MAXIMUM_SPACE (CSC_SIZE_T)0x40000000
#define CSC_LINKED_LIST_MAXIMUM_ELEMENT_SIZE (CSC_SIZE_T)0x10000
#endif

typedef struct _CSC_LinkedListVirtualTable
{
	CSC_IBaseInterfaceVirtualTable baseInterfaceVTable;
	CSC_IContainerVirtualTable containerInterfaceVTable;
	CSC_IIterableVirtualTable iterableInterfaceVTable;
} CSC_LinkedListVirtualTable;

static CSC_PCVOID CSCMETHOD CSC_LinkedListIBaseInterfaceGetInterface(_In_ CONST CSC_IBaseInterface* CONST pThis, _In_ CONST EBaseInterfaceType interfaceType);

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerInitialize(_When_(return == CSC_STATUS_SUCCESS, _Out_) CONST CSC_PVOID pMemoryBaseAddress, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_IAllocator* CONST pIAllocator);
static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerErase(_Inout_ CSC_IContainer* CONST pThis);
static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerDestroy(_Inout_ CSC_IContainer* CONST pThis);
static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerCopy(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST struct _CSC_IContainer* CONST pOther);
static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerMove(_Inout_ CSC_IContainer* CONST pThis, _Inout_ struct _CSC_IContainer* CONST pOther);
static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerInsertRange(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements);
static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerRemoveRange(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements);
static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerSwapValues(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T secondIndex);
static CSC_PVOID CSCMETHOD CSC_LinkedListIContainerAccessElement(_In_ CONST CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T index);
static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerIsValid(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerIsEmpty(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerIsElementContainer(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_SIZE_T CSCMETHOD CSC_LinkedListIContainerGetSize(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_SIZE_T CSCMETHOD CSC_LinkedListIContainerGetElementSize(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_SIZE_T CSCMETHOD CSC_LinkedListIContainerGetMaxElements(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_IAllocator* CSCMETHOD CSC_LinkedListIContainerGetIAllocator(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_IContainerVirtualTable* CSCMETHOD CSC_LinkedListIContainerGetNestedContainerVTable(_In_ CONST struct _CSC_IContainer* CONST pThis);

static CSC_STATUS CSCMETHOD CSC_LinkedListIIterableRegisterIterator(_Inout_ CSC_IIterable* CONST pThis, _In_ CONST CSC_IIterator* CONST pIIterator);
static CSC_STATUS CSCMETHOD CSC_LinkedListIIterableUnregisterIterator(_Inout_ CSC_IIterable* CONST pThis, _In_ CONST CSC_IIterator* CONST pIIterator);
static CSC_PVOID CSCMETHOD CSC_LinkedListIIterableFirstElement(_In_ CONST CSC_IIterable* CONST pThis);
static CSC_PVOID CSCMETHOD CSC_LinkedListIIterableNextElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CONST CSC_SIZE_T currentIndex, _In_ CONST CSC_PCVOID pCurrentElement);
static CSC_PVOID CSCMETHOD CSC_LinkedListIIterableLastElement(_In_ CONST CSC_IIterable* CONST pThis);
static CSC_PVOID CSCMETHOD CSC_LinkedListIIterablePreviousElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CONST CSC_SIZE_T currentIndex, _In_ CONST CSC_PCVOID pCurrentElement);
static CSC_PVOID CSCMETHOD CSC_LinkedListIIterableGetElementAt(_In_ CONST CSC_IIterable* CONST pThis, _In_ CONST CSC_SIZE_T index, _In_opt_ CONST CSC_SIZE_T currentIndex, _In_opt_ CONST CSC_PCVOID pCurrentElement);
static CSC_SIZE_T CSCMETHOD CSC_LinkedListIIterableGetElementCount(_In_ CONST CSC_IIterable* CONST pThis);
static CSC_SIZE_T CSCMETHOD CSC_LinkedListIIterableGetElementSize(_In_ CONST CSC_IIterable* CONST pThis);

static CONST CSC_LinkedListVirtualTable g_linkedListVirtualTable =
{
	{
		CSC_LinkedListIBaseInterfaceGetInterface
	},
	{
		CSC_LinkedListIContainerInitialize,
		CSC_LinkedListIContainerErase,
		CSC_LinkedListIContainerDestroy,
		CSC_LinkedListIContainerCopy,
		CSC_LinkedListIContainerMove,
		CSC_LinkedListIContainerInsertRange,
		CSC_LinkedListIContainerRemoveRange,
		CSC_LinkedListIContainerSwapValues,
		CSC_LinkedListIContainerAccessElement,
		CSC_LinkedListIContainerIsValid,
		CSC_LinkedListIContainerIsEmpty,
		CSC_LinkedListIContainerIsElementContainer,
		CSC_LinkedListIContainerGetSize,
		CSC_LinkedListIContainerGetElementSize,
		CSC_LinkedListIContainerGetMaxElements,
		CSC_LinkedListIContainerGetIAllocator,
		CSC_LinkedListIContainerGetNestedContainerVTable
	},
	{
		CSC_LinkedListIIterableRegisterIterator,
		CSC_LinkedListIIterableUnregisterIterator,
		CSC_LinkedListIIterableFirstElement,
		CSC_LinkedListIIterableNextElement,
		CSC_LinkedListIIterableLastElement,
		CSC_LinkedListIIterablePreviousElement,
		CSC_LinkedListIIterableGetElementAt,
		CSC_LinkedListIIterableGetElementCount,
		CSC_LinkedListIIterableGetElementSize
	}
};


static CSC_STATUS CSCMETHOD CSC_LinkedListZeroMemory(_Out_ CSC_LinkedList* CONST pThis);
static CSC_STATUS CSCMETHOD CSC_LinkedListCopyImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_LinkedList* CONST pSrc);
static CSC_STATUS CSCMETHOD CSC_LinkedListFillImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue);
static CSC_STATUS CSCMETHOD CSC_LinkedListInsertRangeImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements);
static CSC_STATUS CSCMETHOD CSC_LinkedListInsertListCopyImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_LinkedList* CONST pOther);
static CSC_STATUS CSCMETHOD CSC_LinkedListRemoveRangeImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements);
static CSC_STATUS CSCMETHOD CSC_LinkedListPopImpl(_Inout_ CSC_LinkedList* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue, _In_ CONST CSC_BOOLEAN front);
static CSC_STATUS CSCMETHOD CSC_LinkedListRotateImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T rotationCount, _In_ CONST CSC_BOOLEAN forward);
static CSC_STATUS CSCMETHOD CSC_LinkedListChainControl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_BOOLEAN circular);
static CSC_LLNode* CSCMETHOD CSC_LinkedListAccessNodeImpl(_In_ CONST CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T index, _In_ CONST CSC_SIZE_T currentIndex, _In_opt_ CONST CSC_LLNode* CONST pCurrentNode);


static CSC_PVOID CSC_LinkedListGetElementFromLLNode(_In_ CONST CSC_LLNode* CONST pNode)
{
	return (CSC_PVOID)(pNode + (CSC_SIZE_T)1);
}


CSC_STATUS CSCMETHOD CSC_LinkedListInitialize(_When_(return == CSC_STATUS_SUCCESS, _Out_) CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_BOOLEAN circular, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status == CSC_STATUS_SUCCESS)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	status = CSC_LinkedListZeroMemory(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!elementSize || elementSize > CSC_LINKED_LIST_MAXIMUM_ELEMENT_SIZE || !CSC_IAllocatorIsUsable(pIAllocator))
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	pThis->baseInterface.pIBaseInterfaceVirtualTable = (CSC_IBaseInterfaceVirtualTable*)&g_linkedListVirtualTable.baseInterfaceVTable;
	pThis->containerInterface.pIContainerVirtualTable = (CSC_IContainerVirtualTable*)&g_linkedListVirtualTable.containerInterfaceVTable;
	pThis->iterableInterface.pIIterableVirtualTable = (CSC_IIterableVirtualTable*)&g_linkedListVirtualTable.iterableInterfaceVTable;

	pThis->elementSize = elementSize;
	pThis->elementCount = (CSC_SIZE_T)0;
	pThis->pListHead = (CSC_LLNode*)NULL;
	pThis->circular = circular;

	pThis->pIIterator = (CSC_IIterator*)NULL;
	pThis->pIAllocator = pIAllocator;

	pThis->pNestedContainerVTable = pNestedContainerVTable;

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListInitializeWithSize(_When_(return == CSC_STATUS_SUCCESS, _Out_) CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_BOOLEAN circular, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status == CSC_STATUS_SUCCESS)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (!elementSize || elementSize > CSC_LINKED_LIST_MAXIMUM_ELEMENT_SIZE || numOfElements > CSC_LINKED_LIST_MAXIMUM_SPACE / (elementSize + sizeof(CSC_LLNode)) || !pIAllocator || (numOfElements && pNestedContainerVTable && !pDefaultValue))
	{
		CSC_LinkedListZeroMemory(pThis);
		return CSC_STATUS_INVALID_PARAMETER;
	}

	status = CSC_LinkedListInitialize(pThis, elementSize, circular, pIAllocator, pNestedContainerVTable);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!numOfElements)
	{
		return CSC_STATUS_SUCCESS;
	}

	status = CSC_LinkedListFillImpl(pThis, numOfElements, pDefaultValue);

	if (status != CSC_STATUS_SUCCESS)
	{
		CSC_LinkedListDestroy(pThis);
	}

	return status;
}

CSC_STATUS CSCMETHOD CSC_LinkedListInitializeWithCopy(_When_(return == CSC_STATUS_SUCCESS, _Out_) CSC_LinkedList* CONST pThis, _In_ CONST CSC_LinkedList* CONST pSrc)
{
	CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status == CSC_STATUS_SUCCESS)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	status = CSC_LinkedListIsValid(pSrc);

	if (status != CSC_STATUS_SUCCESS)
	{
		CSC_LinkedListZeroMemory(pThis);
		return status;
	}

	status = CSC_LinkedListInitialize(pThis, pSrc->elementSize, pSrc->circular, pSrc->pIAllocator, pSrc->pNestedContainerVTable);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	status = CSC_LinkedListInsertListCopyImpl(pThis, (CSC_SIZE_T)0, pSrc);

	if (status != CSC_STATUS_SUCCESS)
	{
		CSC_LinkedListDestroy(pThis);
	}

	return status;
}

CSC_STATUS CSCMETHOD CSC_LinkedListInitializeWithArray(_When_(return == CSC_STATUS_SUCCESS, _Out_) CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_BOOLEAN circular, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_PCVOID pElements, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status == CSC_STATUS_SUCCESS)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (!elementSize || elementSize > CSC_LINKED_LIST_MAXIMUM_ELEMENT_SIZE || !numOfElements || numOfElements > CSC_LINKED_LIST_MAXIMUM_SPACE / (elementSize + sizeof(CSC_LLNode)) || !pIAllocator || (pNestedContainerVTable && !pElements))
	{
		CSC_LinkedListZeroMemory(pThis);
		return CSC_STATUS_INVALID_PARAMETER;
	}

	status = CSC_LinkedListInitialize(pThis, elementSize, circular, pIAllocator, pNestedContainerVTable);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	status = CSC_LinkedListInsertRangeImpl(pThis, (CSC_SIZE_T)0, numOfElements, pElements);

	if (status != CSC_STATUS_SUCCESS)
	{
		CSC_LinkedListDestroy(pThis);
	}

	return status;
}


CSC_STATUS CSCMETHOD CSC_LinkedListDestroy(_Inout_ CSC_LinkedList* CONST pThis)
{
	CSC_STATUS status = CSC_LinkedListErase(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (pThis->pIIterator)
	{
		CSC_IIteratorOnDestruction(pThis->pIIterator);
	}

	return CSC_LinkedListZeroMemory(pThis);
}

CSC_STATUS CSCMETHOD CSC_LinkedListErase(_Inout_ CSC_LinkedList* CONST pThis)
{
	CSC_PVOID pBuffer;
	CSC_SIZE_T iterator;
	CSC_LLNode* pIterator;
	CSC_IContainer* pIteratorIContainer;
	CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (pThis->elementCount)
	{
		if (!CSC_IAllocatorIsUsable(pThis->pIAllocator))
		{
			return CSC_STATUS_GENERAL_FAILURE;
		}

		pIterator = pThis->pListHead;

		for (iterator = (CSC_SIZE_T)0; iterator < pThis->elementCount; ++iterator)
		{
			if (!pIterator)
			{
				break;
			}

			if (pThis->pNestedContainerVTable)
			{
				pBuffer = CSC_LinkedListGetElementFromLLNode(pIterator);
				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)pBuffer, csc_bit_IContainer);
				pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
			}

			pBuffer = (CSC_PVOID)pIterator;
			pIterator = pIterator->pNext;
			CSC_IAllocatorFree(pThis->pIAllocator, pBuffer);
		}
	}

	pThis->elementCount = (CSC_SIZE_T)0;
	pThis->pListHead = (CSC_LLNode*)NULL;

	if (pThis->pIIterator)
	{
		CSC_IIteratorInvalidateIteration(pThis->pIIterator);
	}

	return CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCMETHOD CSC_LinkedListZeroMemory(_Out_ CSC_LinkedList* CONST pThis)
{
	if (!pThis)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	pThis->baseInterface.pIBaseInterfaceVirtualTable = (CSC_IBaseInterfaceVirtualTable*)NULL;
	pThis->containerInterface.pIContainerVirtualTable = (CSC_IContainerVirtualTable*)NULL;
	pThis->iterableInterface.pIIterableVirtualTable = (CSC_IIterableVirtualTable*)NULL;

	pThis->elementSize = (CSC_SIZE_T)0;
	pThis->elementCount = (CSC_SIZE_T)0;
	pThis->pListHead = (CSC_LLNode*)NULL;
	pThis->circular = (CSC_BOOLEAN)FALSE;

	pThis->pIIterator = (CSC_IIterator*)NULL;
	pThis->pIAllocator = (CONST CSC_IAllocator*)NULL;

	pThis->pNestedContainerVTable = (CONST CSC_IContainerVirtualTable*)NULL;

	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListCopy(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_LinkedList* CONST pSrc)
{
	CSC_STATUS status;
	CSC_LLNode* pNodeBuffer;
	CSC_LinkedList llBuffer;

	if (CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS || CSC_LinkedListIsValid(pSrc) != CSC_STATUS_SUCCESS)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	status = CSC_LinkedListInitialize(&llBuffer, pSrc->elementSize, pSrc->circular, pThis->pIAllocator, pSrc->pNestedContainerVTable);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	status = CSC_LinkedListInsertListCopyImpl(&llBuffer, (CSC_SIZE_T)0, pSrc);

	if (status != CSC_STATUS_SUCCESS)
	{
		CSC_LinkedListDestroy(&llBuffer);
		return status;
	}

	pNodeBuffer = llBuffer.pListHead;

	llBuffer.circular = pThis->circular;
	llBuffer.elementSize = pThis->elementSize;
	llBuffer.pNestedContainerVTable = pThis->pNestedContainerVTable;
	llBuffer.elementCount = pThis->elementCount;
	llBuffer.pListHead = pThis->pListHead;

	pThis->circular = pSrc->circular;
	pThis->elementSize = pSrc->elementSize;
	pThis->pNestedContainerVTable = pSrc->pNestedContainerVTable;
	pThis->elementCount = pSrc->elementCount;
	pThis->pListHead = pNodeBuffer;

	CSC_LinkedListDestroy(&llBuffer);

	if (pThis->pIIterator)
	{
		CSC_IIteratorInvalidateIteration(pThis->pIIterator);
	}

	return CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCMETHOD CSC_LinkedListCopyImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_LinkedList* CONST pSrc)
{
	CSC_STATUS status = CSC_LinkedListIsValid(pSrc);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	status = CSC_LinkedListErase(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	pThis->circular = pSrc->circular;
	pThis->elementSize = pSrc->elementSize;
	pThis->pNestedContainerVTable = pSrc->pNestedContainerVTable;

	if (pSrc->elementCount)
	{
		status = CSC_LinkedListInsertListCopyImpl(pThis, (CSC_SIZE_T)0, pSrc);
	}

	return status;
}

CSC_STATUS CSCMETHOD CSC_LinkedListMove(_Inout_ CSC_LinkedList* CONST pThis, _Inout_ CSC_LinkedList* CONST pSrc)
{
	CSC_STATUS status;

	if (CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS || CSC_LinkedListIsValid(pSrc) != CSC_STATUS_SUCCESS)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pThis == pSrc)
	{
		return CSC_STATUS_SUCCESS;
	}

	if (pThis->pIAllocator == pSrc->pIAllocator)
	{
		status = CSC_LinkedListErase(pThis);

		if (status != CSC_STATUS_SUCCESS)
		{
			return status;
		}

		pThis->circular = pSrc->circular;
		pThis->elementSize = pSrc->elementSize;
		pThis->pNestedContainerVTable = pSrc->pNestedContainerVTable;
		pThis->elementCount = pSrc->elementCount;
		pThis->pListHead = pSrc->pListHead;

		pSrc->elementCount = (CSC_SIZE_T)0;
		pSrc->pListHead = (CSC_LLNode*)NULL;

		if (pSrc->pIIterator)
		{
			CSC_IIteratorInvalidateIteration(pSrc->pIIterator);
		}
	}
	else
	{
		if (pSrc->elementCount)
		{
			status = CSC_LinkedListCopy(pThis, pSrc);

			if (status != CSC_STATUS_SUCCESS)
			{
				return status;
			}
		}
		else
		{
			status = CSC_LinkedListErase(pThis);

			if (status != CSC_STATUS_SUCCESS)
			{
				return status;
			}

			pThis->circular = pSrc->circular;
			pThis->elementSize = pSrc->elementSize;
			pThis->pNestedContainerVTable = pSrc->pNestedContainerVTable;
		}

		CSC_LinkedListErase(pSrc);
	}

	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListFill(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue)
{
	return CSC_LinkedListFillImpl(pThis, numOfElements, pValue);
}

static CSC_STATUS CSCMETHOD CSC_LinkedListFillImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue)
{
	CSC_PVOID pIterator;
	CSC_LinkedList llBuffer;
	CSC_LLNode *pFirst, *pLast, *pOld;
	CSC_IContainer* pIteratorIContainer;
	CONST CSC_IContainer* pElementIContainer;
	CSC_SIZE_T iterator, defaultValueElementSize;
	CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (numOfElements > CSC_LINKED_LIST_MAXIMUM_SPACE / (pThis->elementSize + sizeof(CSC_LLNode)))
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}

	if (!numOfElements || (pThis->pNestedContainerVTable && !pValue))
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	status = CSC_LinkedListInitialize(&llBuffer, pThis->elementSize, pThis->circular, pThis->pIAllocator, pThis->pNestedContainerVTable);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!CSC_IAllocatorIsUsable(llBuffer.pIAllocator))
	{
		CSC_LinkedListDestroy(&llBuffer);
		return CSC_STATUS_GENERAL_FAILURE;
	}

	pOld = (CSC_LLNode*)NULL;
	pFirst = (CSC_LLNode*)NULL;
	pLast = (CSC_LLNode*)NULL;

	if (llBuffer.pNestedContainerVTable)
	{
		pElementIContainer = (CONST CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)pValue, csc_bit_IContainer);

		if (!pElementIContainer || pElementIContainer->pIContainerVirtualTable != llBuffer.pNestedContainerVTable)
		{
			CSC_LinkedListDestroy(&llBuffer);
			return CSC_STATUS_INVALID_PARAMETER;
		}

		defaultValueElementSize = llBuffer.pNestedContainerVTable->pGetElementSize(pElementIContainer);

		if (!defaultValueElementSize)
		{
			CSC_LinkedListDestroy(&llBuffer);
			return CSC_STATUS_INVALID_PARAMETER;
		}

		for (iterator = (CSC_SIZE_T)0; iterator < numOfElements; ++iterator)
		{
			pLast = (CSC_LLNode*)CSC_IAllocatorAllocZero(llBuffer.pIAllocator, (llBuffer.elementSize + sizeof(CSC_LLNode)));

			if (!pLast)
			{
				while (pOld)
				{
					pFirst = pOld;
					pOld = pOld->pPrevious;

					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pFirst), csc_bit_IContainer);
					llBuffer.pNestedContainerVTable->pDestroy(pIteratorIContainer);

					CSC_IAllocatorFree(llBuffer.pIAllocator, (CSC_PVOID)pFirst);
				}

				CSC_LinkedListDestroy(&llBuffer);
				return CSC_STATUS_GENERAL_FAILURE;
			}

			pIterator = CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pLast);
			status = llBuffer.pNestedContainerVTable->pInitialize(pIterator, defaultValueElementSize, llBuffer.pIAllocator);

			if (status == CSC_STATUS_SUCCESS)
			{
				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)pIterator, csc_bit_IContainer);

				if (llBuffer.pNestedContainerVTable->pCopy(pIteratorIContainer, pElementIContainer) != CSC_STATUS_SUCCESS)
				{
					llBuffer.pNestedContainerVTable->pDestroy(pIteratorIContainer);
					CSC_IAllocatorFree(llBuffer.pIAllocator, (CSC_PVOID)pLast);
					pLast = (CSC_LLNode*)NULL;
				}
			}
			else
			{
				CSC_IAllocatorFree(llBuffer.pIAllocator, (CSC_PVOID)pLast);
				pLast = (CSC_LLNode*)NULL;
			}

			if (!pLast)
			{
				while (pOld)
				{
					pFirst = pOld;
					pOld = pOld->pPrevious;

					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pFirst), csc_bit_IContainer);
					llBuffer.pNestedContainerVTable->pDestroy(pIteratorIContainer);

					CSC_IAllocatorFree(llBuffer.pIAllocator, (CSC_PVOID)pFirst);
				}

				CSC_LinkedListDestroy(&llBuffer);
				return CSC_STATUS_GENERAL_FAILURE;
			}

			if (!pFirst)
			{
				pFirst = pLast;
			}

			pLast->pPrevious = pOld;
			pLast->pNext = (CSC_LLNode*)NULL;

			if (pOld)
			{
				pOld->pNext = pLast;
			}

			pOld = pLast;
		}
	}
	else
	{
		for (iterator = (CSC_SIZE_T)0; iterator < numOfElements; ++iterator)
		{
			pLast = (CSC_LLNode*)CSC_IAllocatorAllocZero(llBuffer.pIAllocator, (llBuffer.elementSize + sizeof(CSC_LLNode)));

			if (!pLast)
			{
				while (pOld)
				{
					pFirst = pOld;
					pOld = pOld->pPrevious;
					CSC_IAllocatorFree(llBuffer.pIAllocator, (CSC_PVOID)pFirst);
				}

				CSC_LinkedListDestroy(&llBuffer);
				return CSC_STATUS_GENERAL_FAILURE;
			}

			if (pValue && CSC_MemoryUtilsCopyMemory(CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pLast), pValue, llBuffer.elementSize) != CSC_STATUS_SUCCESS)
			{
				CSC_IAllocatorFree(llBuffer.pIAllocator, (CSC_PVOID)pLast);
				pLast = (CSC_LLNode*)NULL;
			}

			if (!pLast)
			{
				while (pOld)
				{
					pFirst = pOld;
					pOld = pOld->pPrevious;
					CSC_IAllocatorFree(llBuffer.pIAllocator, (CSC_PVOID)pFirst);
				}

				CSC_LinkedListDestroy(&llBuffer);
				return CSC_STATUS_GENERAL_FAILURE;
			}

			if (!pFirst)
			{
				pFirst = pLast;
			}

			pLast->pPrevious = pOld;
			pLast->pNext = (CSC_LLNode*)NULL;

			if (pOld)
			{
				pOld->pNext = pLast;
			}

			pOld = pLast;
		}
	}

	do
	{
		if (pFirst && pLast)
		{
			pLast->pNext = (llBuffer.circular) ? pFirst : (CSC_LLNode*)NULL;
			pFirst->pPrevious = (llBuffer.circular) ? pLast : (CSC_LLNode*)NULL;
			llBuffer.elementCount = numOfElements;
			llBuffer.pListHead = pFirst;
			break;
		}

		while (pOld)
		{
			pFirst = pOld;
			pOld = pOld->pPrevious;

			if (llBuffer.pNestedContainerVTable)
			{
				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pFirst), csc_bit_IContainer);
				llBuffer.pNestedContainerVTable->pDestroy(pIteratorIContainer);
			}

			CSC_IAllocatorFree(llBuffer.pIAllocator, (CSC_PVOID)pFirst);
		}

		CSC_LinkedListDestroy(&llBuffer);
		return CSC_STATUS_GENERAL_FAILURE;

	} while (FALSE);

	pOld = llBuffer.pListHead;

	llBuffer.elementCount = pThis->elementCount;
	llBuffer.pListHead = pThis->pListHead;

	pThis->elementCount = numOfElements;
	pThis->pListHead = pOld;

	CSC_LinkedListDestroy(&llBuffer);

	if (pThis->pIIterator)
	{
		CSC_IIteratorInvalidateIteration(pThis->pIIterator);
	}

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListFillRange(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue)
{
	CSC_PVOID pBuffer;
	CSC_SIZE_T iterator;
	CSC_LLNode* pIterator;
	CSC_LinkedList llBuffer;
	CSC_IContainer* pIteratorIContainer;
	CONST CSC_IContainer* pDefaultValueIContainer;
	CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!numOfElements || firstIndex >= pThis->elementCount || numOfElements > pThis->elementCount - firstIndex)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	status = CSC_LinkedListInitializeWithCopy(&llBuffer, pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	pIterator = CSC_LinkedListAccessNodeImpl(&llBuffer, firstIndex, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);

	if (llBuffer.pNestedContainerVTable)
	{
		if (!pValue)
		{
			CSC_LinkedListDestroy(&llBuffer);
			return CSC_STATUS_INVALID_PARAMETER;
		}

		pDefaultValueIContainer = (CONST CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)pValue, csc_bit_IContainer);

		if (!pDefaultValueIContainer || pDefaultValueIContainer->pIContainerVirtualTable != llBuffer.pNestedContainerVTable)
		{
			CSC_LinkedListDestroy(&llBuffer);
			return CSC_STATUS_INVALID_PARAMETER;
		}

		for (iterator = (CSC_SIZE_T)0; iterator < numOfElements; ++iterator)
		{
			if (!pIterator)
			{
				CSC_LinkedListDestroy(&llBuffer);
				return CSC_STATUS_GENERAL_FAILURE;
			}

			pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pIterator), csc_bit_IContainer);
			status = llBuffer.pNestedContainerVTable->pCopy(pIteratorIContainer, pDefaultValueIContainer);

			if (status != CSC_STATUS_SUCCESS)
			{
				CSC_LinkedListDestroy(&llBuffer);
				return status;
			}

			pIterator = pIterator->pNext;
		}
	}
	else
	{
		for (iterator = (CSC_SIZE_T)0; iterator < numOfElements; ++iterator)
		{
			if (!pIterator)
			{
				CSC_LinkedListDestroy(&llBuffer);
				return CSC_STATUS_GENERAL_FAILURE;
			}

			if (pValue)
			{
				status = CSC_MemoryUtilsCopyMemory(CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pIterator), pValue, llBuffer.elementSize);
			}
			else
			{
				status = CSC_MemoryUtilsSetZeroMemory(CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pIterator), llBuffer.elementSize);
			}

			if (status != CSC_STATUS_SUCCESS)
			{
				CSC_LinkedListDestroy(&llBuffer);
				return status;
			}

			pIterator = pIterator->pNext;
		}
	}

	pIterator = llBuffer.pListHead;
	llBuffer.pListHead = pThis->pListHead;
	pThis->pListHead = pIterator;

	CSC_LinkedListDestroy(&llBuffer);

	if (pThis->pIIterator)
	{
		CSC_IIteratorInvalidateIteration(pThis->pIIterator);
	}

	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListInsertElement(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_opt_ CONST CSC_PCVOID pValue)
{
	return CSC_LinkedListInsertRangeImpl(pThis, insertIndex, (CSC_SIZE_T)1, pValue);
}

CSC_STATUS CSCMETHOD CSC_LinkedListInsertRange(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements)
{
	return CSC_LinkedListInsertRangeImpl(pThis, insertIndex, numOfElements, pElements);
}

static CSC_STATUS CSCMETHOD CSC_LinkedListInsertRangeImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements)
{
	CSC_PVOID pIterator;
	CSC_SIZE_T iterator;
	CSC_IContainer* pIteratorIContainer;
	CONST CSC_IContainer* pElementIContainer;
	CSC_LLNode *pFirst, *pLast, *pOld, *pInsertElement, *pPrevElement;
	CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (numOfElements > (CSC_LINKED_LIST_MAXIMUM_SPACE / (pThis->elementSize + sizeof(CSC_LLNode))) - pThis->elementCount)
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}

	if (insertIndex > pThis->elementCount || !numOfElements || (pThis->pNestedContainerVTable && !pElements))
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pThis->pNestedContainerVTable)
	{
		for (iterator = (CSC_SIZE_T)0; iterator < numOfElements; ++iterator)
		{
			pElementIContainer = (CONST CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pElements + pThis->elementSize * iterator), csc_bit_IContainer);

			if (!pElementIContainer || pElementIContainer->pIContainerVirtualTable != pThis->pNestedContainerVTable)
			{
				return CSC_STATUS_INVALID_PARAMETER;
			}
		}
	}

	if (!CSC_IAllocatorIsUsable(pThis->pIAllocator))
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}

	pInsertElement = (CSC_LLNode*)NULL;
	pPrevElement = (CSC_LLNode*)NULL;

	if (!insertIndex)
	{
		if (insertIndex != pThis->elementCount)
		{
			pInsertElement = pThis->pListHead;

			if (!pInsertElement)
			{
				return CSC_STATUS_GENERAL_FAILURE;
			}
		}
	}
	else
	{
		if (insertIndex == pThis->elementCount)
		{
			pPrevElement = CSC_LinkedListAccessNodeImpl(pThis, pThis->elementCount - (CSC_SIZE_T)1, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);

			if (!pPrevElement)
			{
				return CSC_STATUS_GENERAL_FAILURE;
			}
		}
		else
		{
			pInsertElement = CSC_LinkedListAccessNodeImpl(pThis, insertIndex, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);

			if (!pInsertElement)
			{
				return CSC_STATUS_GENERAL_FAILURE;
			}
		}
	}

	pOld = (CSC_LLNode*)NULL;
	pFirst = (CSC_LLNode*)NULL;
	pLast = (CSC_LLNode*)NULL;

	for (iterator = (CSC_SIZE_T)0; iterator < numOfElements; ++iterator)
	{
		pLast = (CSC_LLNode*)CSC_IAllocatorAllocZero(pThis->pIAllocator, (pThis->elementSize + sizeof(CSC_LLNode)));

		if (!pLast)
		{
			while (pOld)
			{
				pFirst = pOld;
				pOld = pOld->pPrevious;

				if (pThis->pNestedContainerVTable)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pFirst), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
				}

				CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pFirst);
			}

			return CSC_STATUS_GENERAL_FAILURE;
		}

		if (pThis->pNestedContainerVTable)
		{
			pElementIContainer = (CONST CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pElements + pThis->elementSize * iterator), csc_bit_IContainer);
			pIterator = CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pLast);

			status = pThis->pNestedContainerVTable->pInitialize(pIterator, pThis->pNestedContainerVTable->pGetElementSize(pElementIContainer), pThis->pIAllocator);

			if (status == CSC_STATUS_SUCCESS)
			{
				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)pIterator, csc_bit_IContainer);

				if (pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, pElementIContainer) != CSC_STATUS_SUCCESS)
				{
					pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
					CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pLast);
					pLast = (CSC_LLNode*)NULL;
				}
			}
			else
			{
				CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pLast);
				pLast = (CSC_LLNode*)NULL;
			}
		}
		else if (pElements)
		{
			if (CSC_MemoryUtilsCopyMemory(CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pLast), (CSC_PCVOID)((CONST CSC_BYTE* CONST)pElements + iterator * pThis->elementSize), pThis->elementSize) != CSC_STATUS_SUCCESS)
			{
				CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pLast);
				pLast = (CSC_LLNode*)NULL;
			}
		}

		if (!pLast)
		{
			while (pOld)
			{
				pFirst = pOld;
				pOld = pOld->pPrevious;

				if (pThis->pNestedContainerVTable)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pFirst), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
				}

				CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pFirst);
			}

			return CSC_STATUS_GENERAL_FAILURE;
		}

		if (!pFirst)
		{
			pFirst = pLast;
		}

		pLast->pPrevious = pOld;
		pLast->pNext = (CSC_LLNode*)NULL;

		if (pOld)
		{
			pOld->pNext = pLast;
		}

		pOld = pLast;
	}

	do
	{
		if (pFirst && pLast)
		{
			if (!insertIndex)
			{
				if (insertIndex == pThis->elementCount)
				{
					pLast->pNext = (pThis->circular) ? pFirst : (CSC_LLNode*)NULL;
					pFirst->pPrevious = (pThis->circular) ? pLast : (CSC_LLNode*)NULL;
					pThis->pListHead = pFirst;
					break;
				}
				else if (pInsertElement)
				{
					pFirst->pPrevious = pInsertElement->pPrevious;
					pLast->pNext = pInsertElement;

					if (pFirst->pPrevious)
					{
						pFirst->pPrevious->pNext = pFirst;
					}

					pInsertElement->pPrevious = pLast;
					pThis->pListHead = pFirst;
					break;
				}
			}
			else
			{
				if (insertIndex == pThis->elementCount)
				{
					if (pPrevElement)
					{
						pFirst->pPrevious = pPrevElement;
						pLast->pNext = pPrevElement->pNext;

						if (pLast->pNext)
						{
							pLast->pNext->pPrevious = pLast;
						}

						pPrevElement->pNext = pFirst;
						break;
					}
				}
				else if (pInsertElement)
				{
					pFirst->pPrevious = pInsertElement->pPrevious;
					pLast->pNext = pInsertElement;

					pFirst->pPrevious->pNext = pFirst;
					pInsertElement->pPrevious = pLast;
					break;
				}
			}
		}

		while (pOld)
		{
			pFirst = pOld;
			pOld = pOld->pPrevious;

			if (pThis->pNestedContainerVTable)
			{
				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pFirst), csc_bit_IContainer);
				pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
			}

			CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pFirst);
		}

		return CSC_STATUS_GENERAL_FAILURE;

	} while (FALSE);

	pThis->elementCount += numOfElements;

	if (pThis->pIIterator)
	{
		CSC_IIteratorOnInsertion(pThis->pIIterator, insertIndex, numOfElements, pThis->elementCount);
	}

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListInsertListCopy(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_LinkedList* CONST pOther)
{
	return CSC_LinkedListInsertListCopyImpl(pThis, insertIndex, pOther);
}

static CSC_STATUS CSCMETHOD CSC_LinkedListInsertListCopyImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_LinkedList* CONST pOther)
{
	CSC_STATUS status;
	CSC_PVOID pIterator;
	CSC_SIZE_T iterator;
	CSC_IContainer* pIteratorIContainer;
	CONST CSC_IContainer* pIteratorIContainerSrc;
	CSC_LLNode *pFirst, *pLast, *pOld, *pInsertElement, *pPrevElement, *pBuffer;

	if (CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS || CSC_LinkedListIsValid(pOther) != CSC_STATUS_SUCCESS)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pOther->elementCount > (CSC_LINKED_LIST_MAXIMUM_SPACE / (pThis->elementSize + sizeof(CSC_LLNode))) - pThis->elementCount)
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}

	if (insertIndex > pThis->elementCount || pThis->elementSize != pOther->elementSize || pThis->pNestedContainerVTable != pOther->pNestedContainerVTable)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (!pOther->elementCount)
	{
		return CSC_STATUS_SUCCESS;
	}

	if (!CSC_IAllocatorIsUsable(pThis->pIAllocator))
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}

	pInsertElement = (CSC_LLNode*)NULL;
	pPrevElement = (CSC_LLNode*)NULL;

	if (!insertIndex)
	{
		if (insertIndex != pThis->elementCount)
		{
			pInsertElement = pThis->pListHead;

			if (!pInsertElement)
			{
				return CSC_STATUS_GENERAL_FAILURE;
			}
		}
	}
	else
	{
		if (insertIndex == pThis->elementCount)
		{
			pPrevElement = CSC_LinkedListAccessNodeImpl(pThis, pThis->elementCount - (CSC_SIZE_T)1, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);

			if (!pPrevElement)
			{
				return CSC_STATUS_GENERAL_FAILURE;
			}
		}
		else
		{
			pInsertElement = CSC_LinkedListAccessNodeImpl(pThis, insertIndex, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);

			if (!pInsertElement)
			{
				return CSC_STATUS_GENERAL_FAILURE;
			}
		}
	}

	pOld = (CSC_LLNode*)NULL;
	pFirst = (CSC_LLNode*)NULL;
	pLast = (CSC_LLNode*)NULL;
	pBuffer = (CSC_LLNode*)NULL;

	for (iterator = (CSC_SIZE_T)0; iterator < pOther->elementCount; ++iterator)
	{
		pLast = (CSC_LLNode*)CSC_IAllocatorAllocZero(pThis->pIAllocator, (pThis->elementSize + sizeof(CSC_LLNode)));

		if (!pLast)
		{
			while (pOld)
			{
				pFirst = pOld;
				pOld = pOld->pPrevious;

				if (pThis->pNestedContainerVTable)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pFirst), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
				}

				CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pFirst);
			}

			return CSC_STATUS_GENERAL_FAILURE;
		}

		pBuffer = CSC_LinkedListAccessNodeImpl(pOther, iterator, (pBuffer) ? iterator - (CSC_SIZE_T)1 : CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)pBuffer);

		if (!pBuffer)
		{
			CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pLast);
			pLast = (CSC_LLNode*)NULL;
		}
		else
		{
			if (pThis->pNestedContainerVTable)
			{
				pIteratorIContainerSrc = (CONST CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)CSC_LinkedListGetElementFromLLNode(pBuffer), csc_bit_IContainer);
				pIterator = CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pLast);

				status = pThis->pNestedContainerVTable->pInitialize(pIterator, pThis->pNestedContainerVTable->pGetElementSize(pIteratorIContainerSrc), pThis->pIAllocator);

				if (status == CSC_STATUS_SUCCESS)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)pIterator, csc_bit_IContainer);

					if (pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, pIteratorIContainerSrc) != CSC_STATUS_SUCCESS)
					{
						pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
						CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pLast);
						pLast = (CSC_LLNode*)NULL;
					}
				}
				else
				{
					CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pLast);
					pLast = (CSC_LLNode*)NULL;
				}
			}
			else
			{
				if (CSC_MemoryUtilsCopyMemory(CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pLast), CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pBuffer), pThis->elementSize) != CSC_STATUS_SUCCESS)
				{
					CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pLast);
					pLast = (CSC_LLNode*)NULL;
				}
			}
		}

		if (!pLast)
		{
			while (pOld)
			{
				pFirst = pOld;
				pOld = pOld->pPrevious;

				if (pThis->pNestedContainerVTable)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pFirst), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
				}

				CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pFirst);
			}

			return CSC_STATUS_GENERAL_FAILURE;
		}

		if (!pFirst)
		{
			pFirst = pLast;
		}

		pLast->pPrevious = pOld;
		pLast->pNext = (CSC_LLNode*)NULL;

		if (pOld)
		{
			pOld->pNext = pLast;
		}

		pOld = pLast;
	}

	do
	{
		if (pFirst && pLast)
		{
			if (!insertIndex)
			{
				if (insertIndex == pThis->elementCount)
				{
					pLast->pNext = (pThis->circular) ? pFirst : (CSC_LLNode*)NULL;
					pFirst->pPrevious = (pThis->circular) ? pLast : (CSC_LLNode*)NULL;
					pThis->pListHead = pFirst;
					break;
				}
				else if (pInsertElement)
				{
					pFirst->pPrevious = pInsertElement->pPrevious;
					pLast->pNext = pInsertElement;

					if (pFirst->pPrevious)
					{
						pFirst->pPrevious->pNext = pFirst;
					}

					pInsertElement->pPrevious = pLast;
					pThis->pListHead = pFirst;
					break;
				}
			}
			else
			{
				if (insertIndex == pThis->elementCount)
				{
					if (pPrevElement)
					{
						pFirst->pPrevious = pPrevElement;
						pLast->pNext = pPrevElement->pNext;

						if (pLast->pNext)
						{
							pLast->pNext->pPrevious = pLast;
						}

						pPrevElement->pNext = pFirst;
						break;
					}
				}
				else if (pInsertElement)
				{
					pFirst->pPrevious = pInsertElement->pPrevious;
					pLast->pNext = pInsertElement;

					pFirst->pPrevious->pNext = pFirst;
					pInsertElement->pPrevious = pLast;
					break;
				}
			}
		}

		while (pOld)
		{
			pFirst = pOld;
			pOld = pOld->pPrevious;

			if (pThis->pNestedContainerVTable)
			{
				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pFirst), csc_bit_IContainer);
				pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
			}

			CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pFirst);
		}

		return CSC_STATUS_GENERAL_FAILURE;

	} while (FALSE);

	pThis->elementCount += pOther->elementCount;

	if (pThis->pIIterator)
	{
		CSC_IIteratorOnInsertion(pThis->pIIterator, insertIndex, pOther->elementCount, pThis->elementCount);
	}

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListInsertListMove(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _Inout_ CSC_LinkedList* CONST pOther)
{
	CSC_SIZE_T otherElementCount;
	CSC_LLNode *pInsertElement, *pPrevElement, *pOtherFirstElement, *pOtherLastElement;
	CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (pThis == pOther)
	{
		return CSC_LinkedListInsertListCopyImpl(pThis, insertIndex, pOther);
	}

	status = CSC_LinkedListIsValid(pOther);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (pThis->pIAllocator == pOther->pIAllocator)
	{
		if (insertIndex > pThis->elementCount || pThis->elementSize != pOther->elementSize || pThis->pNestedContainerVTable != pOther->pNestedContainerVTable)
		{
			return CSC_STATUS_INVALID_PARAMETER;
		}

		otherElementCount = pOther->elementCount;

		if (otherElementCount > (CSC_LINKED_LIST_MAXIMUM_SPACE / (pThis->elementSize + sizeof(CSC_LLNode))) - pThis->elementCount)
		{
			return CSC_STATUS_GENERAL_FAILURE;
		}

		if (!otherElementCount)
		{
			return CSC_STATUS_SUCCESS;
		}

		pOtherFirstElement = CSC_LinkedListAccessNodeImpl(pOther, (CSC_SIZE_T)0, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);
		pOtherLastElement = CSC_LinkedListAccessNodeImpl(pOther, otherElementCount - (CSC_SIZE_T)1, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);

		if (!pOtherFirstElement || !pOtherLastElement)
		{
			return CSC_STATUS_GENERAL_FAILURE;
		}

		if (!insertIndex)
		{
			if (insertIndex != pThis->elementCount)
			{
				pInsertElement = pThis->pListHead;

				if (!pInsertElement)
				{
					return CSC_STATUS_GENERAL_FAILURE;
				}

				pOtherFirstElement->pPrevious = pInsertElement->pPrevious;
				pOtherLastElement->pNext = pInsertElement;

				if (pOtherFirstElement->pPrevious)
				{
					pOtherFirstElement->pPrevious->pNext = pOtherFirstElement;
				}

				pInsertElement->pPrevious = pOtherLastElement;
			}
			else
			{
				if (pThis->circular != pOther->circular)
				{
					pOtherLastElement->pNext = (pThis->circular) ? pOtherFirstElement : (CSC_LLNode*)NULL;
					pOtherFirstElement->pPrevious = (pThis->circular) ? pOtherLastElement : (CSC_LLNode*)NULL;
				}
			}

			pThis->pListHead = pOtherFirstElement;
		}
		else
		{
			if (insertIndex == pThis->elementCount)
			{
				pPrevElement = CSC_LinkedListAccessNodeImpl(pThis, pThis->elementCount - (CSC_SIZE_T)1, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);

				if (!pPrevElement)
				{
					return CSC_STATUS_GENERAL_FAILURE;
				}

				pOtherFirstElement->pPrevious = pPrevElement;
				pOtherLastElement->pNext = pPrevElement->pNext;

				if (pOtherLastElement->pNext)
				{
					pOtherLastElement->pNext->pPrevious = pOtherLastElement;
				}

				pPrevElement->pNext = pOtherFirstElement;
			}
			else
			{
				pInsertElement = CSC_LinkedListAccessNodeImpl(pThis, insertIndex, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);

				if (!pInsertElement)
				{
					return CSC_STATUS_GENERAL_FAILURE;
				}

				pOtherFirstElement->pPrevious = pInsertElement->pPrevious;
				pOtherLastElement->pNext = pInsertElement;

				pOtherFirstElement->pPrevious->pNext = pOtherFirstElement;
				pInsertElement->pPrevious = pOtherLastElement;
			}
		}

		pThis->elementCount += otherElementCount;
		pOther->elementCount = (CSC_SIZE_T)0;
		pOther->pListHead = (CSC_LLNode*)NULL;

		if (pThis->pIIterator)
		{
			CSC_IIteratorOnInsertion(pThis->pIIterator, insertIndex, otherElementCount, pThis->elementCount);
		}

		if (pOther->pIIterator)
		{
			CSC_IIteratorInvalidateIteration(pOther->pIIterator);
		}
	}
	else
	{
		status = CSC_LinkedListInsertListCopyImpl(pThis, insertIndex, pOther);

		if (status != CSC_STATUS_SUCCESS)
		{
			return status;
		}

		CSC_LinkedListErase(pOther);
	}

	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListRemoveElement(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex)
{
	return CSC_LinkedListRemoveRangeImpl(pThis, removeIndex, (CSC_SIZE_T)1);
}

CSC_STATUS CSCMETHOD CSC_LinkedListRemoveRange(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements)
{
	return CSC_LinkedListRemoveRangeImpl(pThis, removeIndex, numOfElements);
}

static CSC_STATUS CSCMETHOD CSC_LinkedListRemoveRangeImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements)
{
	CSC_PVOID pBuffer;
	CSC_SIZE_T iterator;
	CSC_IContainer* pIteratorIContainer;
	CSC_LLNode *pFirst, *pLast, *pIterator;
	CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!numOfElements || removeIndex >= pThis->elementCount || numOfElements > pThis->elementCount - removeIndex)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (numOfElements == pThis->elementCount)
	{
		return CSC_LinkedListErase(pThis);
	}

	if (!CSC_IAllocatorIsUsable(pThis->pIAllocator))
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}

	pFirst = CSC_LinkedListAccessNodeImpl(pThis, removeIndex, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);

	if (!pFirst)
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}

	pLast = CSC_LinkedListAccessNodeImpl(pThis, removeIndex + numOfElements - (CSC_SIZE_T)1, removeIndex, (CONST CSC_LLNode* CONST)pFirst);

	if (!pLast)
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}

	if (!removeIndex)
	{
		pThis->pListHead = pLast->pNext;
	}

	if (pFirst->pPrevious)
	{
		pFirst->pPrevious->pNext = pLast->pNext;
	}

	if (pLast->pNext)
	{
		pLast->pNext->pPrevious = pFirst->pPrevious;
	}

	pFirst->pPrevious = (CSC_LLNode*)NULL;
	pLast->pNext = (CSC_LLNode*)NULL;

	pThis->elementCount -= numOfElements;
	pIterator = pFirst;

	for (iterator = (CSC_SIZE_T)0; iterator < numOfElements; ++iterator)
	{
		if (!pIterator)
		{
			break;
		}

		if (pThis->pNestedContainerVTable)
		{
			pBuffer = CSC_LinkedListGetElementFromLLNode(pIterator);
			pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)pBuffer, csc_bit_IContainer);
			pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
		}

		pBuffer = (CSC_PVOID)pIterator;
		pIterator = pIterator->pNext;
		CSC_IAllocatorFree(pThis->pIAllocator, pBuffer);
	}

	if (pThis->pIIterator)
	{
		CSC_IIteratorOnRemoval(pThis->pIIterator, removeIndex, numOfElements, pThis->elementCount);
	}

	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListPushElement(_Inout_ CSC_LinkedList* CONST pThis, _In_opt_ CONST CSC_PCVOID pValue)
{
	CONST CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	return CSC_LinkedListInsertRangeImpl(pThis, pThis->elementCount, (CSC_SIZE_T)1, pValue);
}

CSC_STATUS CSCMETHOD CSC_LinkedListPopElement(_Inout_ CSC_LinkedList* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue)
{
	return CSC_LinkedListPopImpl(pThis, pValue, (CSC_BOOLEAN)FALSE);
}

CSC_STATUS CSCMETHOD CSC_LinkedListPopFront(_Inout_ CSC_LinkedList* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue)
{
	return CSC_LinkedListPopImpl(pThis, pValue, (CSC_BOOLEAN)TRUE);
}

static CSC_STATUS CSCMETHOD CSC_LinkedListPopImpl(_Inout_ CSC_LinkedList* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue, _In_ CONST CSC_BOOLEAN front)
{
	CSC_PVOID pElement;
	CSC_LLNode* pElementNode;
	CSC_SIZE_T removeIndex;
	CSC_IContainer* pOutputIContainer;
	CSC_IContainer* pElementIContainer;
	CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS || !pThis->elementCount)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	removeIndex = ((front) ? (CSC_SIZE_T)0 : pThis->elementCount - (CSC_SIZE_T)1);

	if (pValue)
	{
		if (!CSC_IAllocatorIsUsable(pThis->pIAllocator))
		{
			CSC_MemoryUtilsSetZeroMemory(pValue, pThis->elementSize);
			return CSC_STATUS_GENERAL_FAILURE;
		}

		pElementNode = CSC_LinkedListAccessNodeImpl(pThis, removeIndex, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);
		
		if (!pElementNode)
		{
			CSC_MemoryUtilsSetZeroMemory(pValue, pThis->elementSize);
			return CSC_STATUS_GENERAL_FAILURE;
		}

		pElement = CSC_LinkedListGetElementFromLLNode((CONST CSC_LLNode* CONST)pElementNode);

		if (pThis->pNestedContainerVTable)
		{
			pElementIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)pElement, csc_bit_IContainer);
			status = pThis->pNestedContainerVTable->pInitialize(pValue, pThis->pNestedContainerVTable->pGetElementSize((CONST CSC_IContainer*)pElementIContainer), pThis->pIAllocator);

			if (status != CSC_STATUS_SUCCESS)
			{
				CSC_MemoryUtilsSetZeroMemory(pValue, pThis->elementSize);
				return CSC_STATUS_GENERAL_FAILURE;
			}

			pOutputIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)pValue, csc_bit_IContainer);
			status = pThis->pNestedContainerVTable->pCopy(pOutputIContainer, (CONST CSC_IContainer*)pElementIContainer);

			if (status != CSC_STATUS_SUCCESS)
			{
				pThis->pNestedContainerVTable->pDestroy(pOutputIContainer);
				return status;
			}

			if (pThis->elementCount == (CSC_SIZE_T)1)
			{
				status = CSC_LinkedListErase(pThis);

				if (status != CSC_STATUS_SUCCESS)
				{
					pThis->pNestedContainerVTable->pDestroy(pOutputIContainer);
				}

				return status;
			}

			if (!removeIndex)
			{
				pThis->pListHead = pElementNode->pNext;
			}

			if (pElementNode->pPrevious)
			{
				pElementNode->pPrevious->pNext = pElementNode->pNext;
			}

			if (pElementNode->pNext)
			{
				pElementNode->pNext->pPrevious = pElementNode->pPrevious;
			}

			pThis->pNestedContainerVTable->pDestroy(pElementIContainer);
		}
		else
		{
			status = CSC_MemoryUtilsCopyMemory(pValue, (CSC_PCVOID)pElement, pThis->elementSize);

			if (status != CSC_STATUS_SUCCESS)
			{
				CSC_MemoryUtilsSetZeroMemory(pValue, pThis->elementSize);
				return status;
			}

			if (pThis->elementCount == (CSC_SIZE_T)1)
			{
				status = CSC_LinkedListErase(pThis);

				if (status != CSC_STATUS_SUCCESS)
				{
					CSC_MemoryUtilsSetZeroMemory(pValue, pThis->elementSize);
				}

				return status;
			}

			if (!removeIndex)
			{
				pThis->pListHead = pElementNode->pNext;
			}

			if (pElementNode->pPrevious)
			{
				pElementNode->pPrevious->pNext = pElementNode->pNext;
			}

			if (pElementNode->pNext)
			{
				pElementNode->pNext->pPrevious = pElementNode->pPrevious;
			}
		}

		--pThis->elementCount;

		CSC_IAllocatorFree(pThis->pIAllocator, (CSC_PVOID)pElementNode);

		if (pThis->pIIterator)
		{
			CSC_IIteratorOnRemoval(pThis->pIIterator, removeIndex, (CSC_SIZE_T)1, pThis->elementCount);
		}

		return CSC_STATUS_SUCCESS;
	}
	
	return CSC_LinkedListRemoveRangeImpl(pThis, removeIndex, (CSC_SIZE_T)1);
}


CSC_STATUS CSCMETHOD CSC_LinkedListAppendCopy(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_LinkedList* CONST pOther)
{
	CONST CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	return CSC_LinkedListInsertListCopyImpl(pThis, pThis->elementCount, pOther);
}

CSC_STATUS CSCMETHOD CSC_LinkedListAppendMove(_Inout_ CSC_LinkedList* CONST pThis, _Inout_ CSC_LinkedList* CONST pOther)
{
	CONST CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	return CSC_LinkedListInsertListMove(pThis, pThis->elementCount, pOther);
}


CSC_STATUS CSCMETHOD CSC_LinkedListRotateForward(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T rotationCount)
{
	return CSC_LinkedListRotateImpl(pThis, rotationCount, (CSC_BOOLEAN)TRUE);
}

CSC_STATUS CSCMETHOD CSC_LinkedListRotateForwardByOne(_Inout_ CSC_LinkedList* CONST pThis)
{
	return CSC_LinkedListRotateImpl(pThis, (CSC_SIZE_T)1, (CSC_BOOLEAN)TRUE);
}

CSC_STATUS CSCMETHOD CSC_LinkedListRotateBackward(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T rotationCount)
{
	return CSC_LinkedListRotateImpl(pThis, rotationCount, (CSC_BOOLEAN)FALSE);
}

CSC_STATUS CSCMETHOD CSC_LinkedListRotateBackwardByOne(_Inout_ CSC_LinkedList* CONST pThis)
{
	return CSC_LinkedListRotateImpl(pThis, (CSC_SIZE_T)1, (CSC_BOOLEAN)FALSE);
}

CSC_STATUS CSCMETHOD CSC_LinkedListRotateToIndex(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T newHeadIndex)
{
	CONST CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!pThis->circular || newHeadIndex >= pThis->elementCount)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (!newHeadIndex)
	{
		return CSC_STATUS_SUCCESS;
	}

	if (newHeadIndex > (pThis->elementCount >> (CSC_SIZE_T)1))
	{
		return CSC_LinkedListRotateForward(pThis, newHeadIndex);
	}
	else
	{
		return CSC_LinkedListRotateBackward(pThis, pThis->elementCount - newHeadIndex);
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListRotateImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T rotationCount, _In_ CONST CSC_BOOLEAN forward)
{
	CSC_LLNode* pIterator;
	CSC_SIZE_T iterator, effectiveRotationCount;
	CONST CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (pThis->elementCount <= (CSC_SIZE_T)1 || !pThis->circular || !rotationCount)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	pIterator = pThis->pListHead;

	if (!pIterator)
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}

	effectiveRotationCount = rotationCount % pThis->elementCount;

	if (!effectiveRotationCount)
	{
		return CSC_STATUS_SUCCESS;
	}

	for (iterator = (CSC_SIZE_T)0; iterator < effectiveRotationCount; ++iterator)
	{
		pIterator = ((forward) ? pIterator->pNext : pIterator->pPrevious);

		if (!pIterator)
		{
			return CSC_STATUS_GENERAL_FAILURE;
		}
	}

	pThis->pListHead = pIterator;

	if (pThis->pIIterator)
	{
		CSC_IIteratorInvalidateIteration(pThis->pIIterator);
	}

	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListOpen(_Inout_ CSC_LinkedList* CONST pThis)
{
	return CSC_LinkedListChainControl(pThis, (CSC_BOOLEAN)FALSE);
}

CSC_STATUS CSCMETHOD CSC_LinkedListClose(_Inout_ CSC_LinkedList* CONST pThis)
{
	return CSC_LinkedListChainControl(pThis, (CSC_BOOLEAN)TRUE);
}

static CSC_STATUS CSCMETHOD CSC_LinkedListChainControl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_BOOLEAN circular)
{
	CSC_LLNode *pFirst, *pLast;
	CONST CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if ((pThis->circular && circular) || (!pThis->circular && !circular))
	{
		return CSC_STATUS_SUCCESS;
	}

	if (pThis->elementCount)
	{
		pFirst = pThis->pListHead;
		pLast = CSC_LinkedListAccessNodeImpl(pThis, pThis->elementCount - (CSC_SIZE_T)1, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);

		if (!pFirst || !pLast)
		{
			return CSC_STATUS_GENERAL_FAILURE;
		}

		pFirst->pPrevious = (circular) ? pLast : (CSC_LLNode*)NULL;
		pLast->pNext = (circular) ? pFirst : (CSC_LLNode*)NULL;
	}

	pThis->circular = circular;

	return CSC_STATUS_SUCCESS;
}


CSC_PVOID CSCMETHOD CSC_LinkedListAccessElement(_In_ CONST CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T index)
{
	CONST CSC_LLNode* pNodeBuffer;

	if (CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS || index >= pThis->elementCount)
	{
		return NULL;
	}

	pNodeBuffer = (CONST CSC_LLNode*)CSC_LinkedListAccessNodeImpl(pThis, index, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);

	return (pNodeBuffer) ? CSC_LinkedListGetElementFromLLNode(pNodeBuffer) : NULL;
}

static CSC_LLNode* CSCMETHOD CSC_LinkedListAccessNodeImpl(_In_ CONST CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T index, _In_ CONST CSC_SIZE_T currentIndex, _In_opt_ CONST CSC_LLNode* CONST pCurrentNode)
{
	CSC_LLNode* pNode;
	CSC_BOOLEAN forwardFromHead, forwardFromCurrentElement;
	CSC_SIZE_T iterator, distanceToHead, distanceToCurrentElement;

	if (!pThis || !pThis->elementCount || !pThis->pListHead || index >= pThis->elementCount)
	{
		return (CSC_LLNode*)NULL;
	}

	forwardFromHead = (pThis->circular && index > (pThis->elementCount >> (CSC_SIZE_T)1)) ? (CSC_BOOLEAN)FALSE : (CSC_BOOLEAN)TRUE;
	distanceToHead =  (forwardFromHead) ? index : (pThis->elementCount - index);

	if (currentIndex != CSC_CONTAINER_INVALID_INDEX && pCurrentNode && currentIndex < pThis->elementCount)
	{
		forwardFromCurrentElement = (index > currentIndex) ? (CSC_BOOLEAN)TRUE : (CSC_BOOLEAN)FALSE;
		distanceToCurrentElement = (forwardFromCurrentElement) ? (index - currentIndex) : (currentIndex - index);

		if (pThis->circular && distanceToCurrentElement > (pThis->elementCount >> (CSC_SIZE_T)1))
		{
			distanceToCurrentElement = pThis->elementCount - distanceToCurrentElement;
			forwardFromCurrentElement = (forwardFromCurrentElement) ? (CSC_BOOLEAN)FALSE : (CSC_BOOLEAN)TRUE;
		}

		if (distanceToCurrentElement < distanceToHead)
		{
			pNode = (CSC_LLNode*)pCurrentNode;

			for (iterator = (CSC_SIZE_T)0; iterator < distanceToCurrentElement; ++iterator)
			{
				if (!pNode)
				{
					return (CSC_LLNode*)NULL;
				}

				pNode = (forwardFromCurrentElement) ? pNode->pNext : pNode->pPrevious;
			}
		}
		else
		{
			pNode = pThis->pListHead;

			for (iterator = (CSC_SIZE_T)0; iterator < distanceToHead; ++iterator)
			{
				if (!pNode)
				{
					return (CSC_LLNode*)NULL;
				}

				pNode = (forwardFromHead) ? pNode->pNext : pNode->pPrevious;
			}
		}
	}
	else
	{
		pNode = pThis->pListHead;

		for (iterator = (CSC_SIZE_T)0; iterator < distanceToHead; ++iterator)
		{
			if (!pNode)
			{
				return (CSC_LLNode*)NULL;
			}

			pNode = (forwardFromHead) ? pNode->pNext : pNode->pPrevious;
		}
	}

	return pNode;
}


CSC_STATUS CSCMETHOD CSC_LinkedListReverse(_Inout_ CSC_LinkedList* CONST pThis)
{
	CSC_SIZE_T iterator;
	CSC_LLNode *pNode, *pNodeBuffer;
	CONST CSC_STATUS status = CSC_LinkedListIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!pThis->pListHead)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	pNode = pThis->pListHead;
	pThis->pListHead = CSC_LinkedListAccessNodeImpl(pThis, pThis->elementCount - (CSC_SIZE_T)1, CSC_CONTAINER_INVALID_INDEX, (CONST CSC_LLNode* CONST)NULL);

	if (!pThis->pListHead)
	{
		pThis->pListHead = pNode;
		return CSC_STATUS_GENERAL_FAILURE;
	}

	for (iterator = (CSC_SIZE_T)0; iterator < pThis->elementCount; ++iterator)
	{
		if (!pNode)
		{
			if (pThis->pIIterator)
			{
				CSC_IIteratorInvalidateIteration(pThis->pIIterator);
			}

			return CSC_STATUS_GENERAL_FAILURE;
		}

		pNodeBuffer = pNode->pNext;
		pNode->pNext = pNode->pPrevious;
		pNode->pPrevious = pNodeBuffer;

		pNode = pNode->pPrevious;
	}

	if (pThis->pIIterator)
	{
		CSC_IIteratorInvalidateIteration(pThis->pIIterator);
	}

	return CSC_STATUS_SUCCESS;
}


CSC_PVOID CSCMETHOD CSC_LinkedListFront(_In_ CONST CSC_LinkedList* CONST pThis)
{
	if (CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS || !pThis->elementCount)
	{
		return NULL;
	}
	else
	{
		return CSC_LinkedListGetElementFromLLNode(pThis->pListHead);
	}
}

CSC_PVOID CSCMETHOD CSC_LinkedListBack(_In_ CONST CSC_LinkedList* CONST pThis)
{
	if (!pThis || !pThis->elementCount)
	{
		return NULL;
	}
	else
	{
		return CSC_LinkedListAccessElement(pThis, pThis->elementCount - (CSC_SIZE_T)1);
	}
}


CSC_STATUS CSCMETHOD CSC_LinkedListIsEmpty(_In_ CONST CSC_LinkedList* CONST pThis)
{
	if (CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return ((pThis->elementCount && pThis->pListHead) ? CSC_STATUS_GENERAL_FAILURE : CSC_STATUS_SUCCESS);
	}
}

CSC_STATUS CSCMETHOD CSC_LinkedListIsValid(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (!pThis || !pThis->elementSize || pThis->elementSize > CSC_LINKED_LIST_MAXIMUM_ELEMENT_SIZE || pThis->baseInterface.pIBaseInterfaceVirtualTable != &g_linkedListVirtualTable.baseInterfaceVTable || pThis->containerInterface.pIContainerVirtualTable != &g_linkedListVirtualTable.containerInterfaceVTable || pThis->iterableInterface.pIIterableVirtualTable != &g_linkedListVirtualTable.iterableInterfaceVTable || !pThis->pIAllocator || (pThis->elementCount && !pThis->pListHead) || (pThis->pListHead && !pThis->elementCount) || pThis->elementCount > CSC_LINKED_LIST_MAXIMUM_SPACE / (pThis->elementSize + sizeof(CSC_LLNode))) ? CSC_STATUS_INVALID_PARAMETER : CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListIsCircular(_In_ CONST CSC_LinkedList* CONST pThis)
{
	if (CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return (pThis->circular) ? CSC_STATUS_GENERAL_FAILURE : CSC_STATUS_SUCCESS;
	}
}


CSC_SIZE_T CSCMETHOD CSC_LinkedListGetSize(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return ((CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS) ? CSC_CONTAINER_INVALID_LENGTH : pThis->elementCount);
}

CSC_SIZE_T CSCMETHOD CSC_LinkedListGetMaxElements(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return ((CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS) ? CSC_CONTAINER_INVALID_LENGTH : CSC_LINKED_LIST_MAXIMUM_SPACE / (pThis->elementSize + sizeof(CSC_LLNode)));
}

CSC_SIZE_T CSCMETHOD CSC_LinkedListGetElementSize(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return ((CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS) ? (CSC_SIZE_T)0 : pThis->elementSize);
}


CSC_IBaseInterface* CSCMETHOD CSC_LinkedListGetIBaseInterface(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (CSC_IBaseInterface*)((CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS) ? NULL : &pThis->baseInterface);
}

CSC_IContainer* CSCMETHOD CSC_LinkedListGetIContainer(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (CSC_IContainer*)((CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS) ? NULL : &pThis->containerInterface);
}

CSC_IIterable* CSCMETHOD CSC_LinkedListGetIIterable(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (CSC_IIterable*)((CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS) ? NULL : &pThis->iterableInterface);
}

CSC_IAllocator* CSCMETHOD CSC_LinkedListGetIAllocator(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (CSC_IAllocator*)((CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS) ? NULL : pThis->pIAllocator);
}

CSC_IContainerVirtualTable* CSCMETHOD CSC_LinkedListGetNestedContainerVTable(_In_ CONST CSC_LinkedList* CONST pThis)
{
	return (CSC_IContainerVirtualTable*)((CSC_LinkedListIsValid(pThis) != CSC_STATUS_SUCCESS) ? NULL : pThis->pNestedContainerVTable);
}


static CSC_PCVOID CSCMETHOD CSC_LinkedListIBaseInterfaceGetInterface(_In_ CONST CSC_IBaseInterface* CONST pThis, _In_ CONST EBaseInterfaceType interfaceType)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)pThis;

	if (!pLinkedList || CSC_LinkedListIsValid(pLinkedList) != CSC_STATUS_SUCCESS)
	{
		return (CSC_PCVOID)NULL;
	}

	switch (interfaceType)
	{
	case csc_bit_IContainer:

		return (CSC_PCVOID)&pLinkedList->containerInterface;

	case csc_bit_IIterable:

		return (CSC_PCVOID)&pLinkedList->iterableInterface;

	default:

		return (CSC_PCVOID)NULL;
	}

	return (CSC_PCVOID)NULL;
}


static CSC_LinkedList* CSCMETHOD CSC_LinkedListIContainerGetObjectPointer(_In_ CONST CSC_IContainer* CONST pThis)
{
	if (!pThis || pThis->pIContainerVirtualTable != &g_linkedListVirtualTable.containerInterfaceVTable)
	{
		return (CSC_LinkedList*)NULL;
	}
	else
	{
		return (CSC_LinkedList*)((CONST CSC_BYTE* CONST)pThis - (CSC_SIZE_T)offsetof(CSC_LinkedList, containerInterface));
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerInitialize(_When_(return == CSC_STATUS_SUCCESS, _Out_) CONST CSC_PVOID pMemoryBaseAddress, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_IAllocator* CONST pIAllocator)
{
	if (!pMemoryBaseAddress || !elementSize || !pIAllocator)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_LinkedListInitialize((CSC_LinkedList* CONST)pMemoryBaseAddress, elementSize, (CSC_BOOLEAN)FALSE, pIAllocator, (CONST CSC_IContainerVirtualTable* CONST)NULL);
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerErase(_Inout_ CSC_IContainer* CONST pThis)
{
	CSC_LinkedList* CONST pLinkedList = (CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_LinkedListErase(pLinkedList);
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerDestroy(_Inout_ CSC_IContainer* CONST pThis)
{
	CSC_LinkedList* CONST pLinkedList = (CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_LinkedListDestroy(pLinkedList);
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerCopy(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST struct _CSC_IContainer* CONST pOther)
{
	CSC_LinkedList* CONST pLinkedList = (CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);
	CONST CSC_LinkedList* CONST pOtherLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pOther);

	if (!pLinkedList || !pOtherLinkedList)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_LinkedListCopy(pLinkedList, pOtherLinkedList);
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerMove(_Inout_ CSC_IContainer* CONST pThis, _Inout_ struct _CSC_IContainer* CONST pOther)
{
	CSC_LinkedList* CONST pLinkedList = (CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);
	CSC_LinkedList* CONST pOtherLinkedList = (CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pOther);

	if (!pLinkedList || !pOtherLinkedList)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_LinkedListMove(pLinkedList, pOtherLinkedList);
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerInsertRange(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements)
{
	CSC_LinkedList* CONST pLinkedList = (CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);

	if (!pLinkedList || !numOfElements)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_LinkedListInsertRange(pLinkedList, insertIndex, numOfElements, pElements);
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerRemoveRange(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements)
{
	CSC_LinkedList* CONST pLinkedList = (CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);

	if (!pLinkedList || !numOfElements)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_LinkedListRemoveRange(pLinkedList, removeIndex, numOfElements);
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerSwapValues(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T secondIndex)
{
	return CSC_STATUS_SUCCESS;
}

static CSC_PVOID CSCMETHOD CSC_LinkedListIContainerAccessElement(_In_ CONST CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T index)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return NULL;
	}
	else
	{
		return CSC_LinkedListAccessElement(pLinkedList, index);
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerIsValid(_In_ CONST CSC_IContainer* CONST pThis)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_LinkedListIsValid(pLinkedList);
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerIsEmpty(_In_ CONST CSC_IContainer* CONST pThis)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_LinkedListIsEmpty(pLinkedList);
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerIsElementContainer(_In_ CONST CSC_IContainer* CONST pThis)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return (CSC_LinkedListGetNestedContainerVTable(pLinkedList) != (CSC_IContainerVirtualTable*)NULL) ? CSC_STATUS_SUCCESS : CSC_STATUS_GENERAL_FAILURE;
	}
}

static CSC_SIZE_T CSCMETHOD CSC_LinkedListIContainerGetSize(_In_ CONST CSC_IContainer* CONST pThis)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return CSC_CONTAINER_INVALID_LENGTH;
	}
	else
	{
		return CSC_LinkedListGetSize(pLinkedList);
	}
}

static CSC_SIZE_T CSCMETHOD CSC_LinkedListIContainerGetElementSize(_In_ CONST CSC_IContainer* CONST pThis)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return (CSC_SIZE_T)0;
	}
	else
	{
		return CSC_LinkedListGetElementSize(pLinkedList);
	}
}

static CSC_SIZE_T CSCMETHOD CSC_LinkedListIContainerGetMaxElements(_In_ CONST CSC_IContainer* CONST pThis)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return CSC_CONTAINER_INVALID_LENGTH;
	}
	else
	{
		return CSC_LinkedListGetMaxElements(pLinkedList);
	}
}

static CSC_IAllocator* CSCMETHOD CSC_LinkedListIContainerGetIAllocator(_In_ CONST CSC_IContainer* CONST pThis)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return (CSC_IAllocator*)NULL;
	}
	else
	{
		return CSC_LinkedListGetIAllocator(pLinkedList);
	}
}

static CSC_IContainerVirtualTable* CSCMETHOD CSC_LinkedListIContainerGetNestedContainerVTable(_In_ CONST struct _CSC_IContainer* CONST pThis)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIContainerGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return (CSC_IContainerVirtualTable*)NULL;
	}
	else
	{
		return CSC_LinkedListGetNestedContainerVTable(pLinkedList);
	}
}


static CSC_LinkedList* CSCMETHOD CSC_LinkedListIIterableGetObjectPointer(_In_ CONST  CSC_IIterable* CONST pThis)
{
	if (!pThis || pThis->pIIterableVirtualTable != &g_linkedListVirtualTable.iterableInterfaceVTable)
	{
		return (CSC_LinkedList*)NULL;
	}
	else
	{
		return (CSC_LinkedList*)((CONST CSC_BYTE* CONST)pThis - (CSC_SIZE_T)offsetof(CSC_LinkedList, iterableInterface));
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListIIterableRegisterIterator(_Inout_ CSC_IIterable* CONST pThis, _In_ CONST CSC_IIterator* CONST pIIterator)
{
	CSC_LinkedList* CONST pLinkedList = (CSC_LinkedList* CONST)CSC_LinkedListIIterableGetObjectPointer(pThis);

	if (!pIIterator || !pLinkedList || CSC_LinkedListIsValid(pLinkedList) != CSC_STATUS_SUCCESS)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pLinkedList->pIIterator)
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}
	else
	{
		pLinkedList->pIIterator = (CSC_IIterator*)pIIterator;
		return CSC_STATUS_SUCCESS;
	}
}

static CSC_STATUS CSCMETHOD CSC_LinkedListIIterableUnregisterIterator(_Inout_ CSC_IIterable* CONST pThis, _In_ CONST CSC_IIterator* CONST pIIterator)
{
	CSC_LinkedList* CONST pLinkedList = (CSC_LinkedList* CONST)CSC_LinkedListIIterableGetObjectPointer(pThis);

	if (!pLinkedList || CSC_LinkedListIsValid(pLinkedList) != CSC_STATUS_SUCCESS || !pLinkedList->pIIterator || pLinkedList->pIIterator != pIIterator)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		pLinkedList->pIIterator = (CSC_IIterator*)NULL;
		return CSC_STATUS_SUCCESS;
	}
}

static CSC_PVOID CSCMETHOD CSC_LinkedListIIterableFirstElement(_In_ CONST CSC_IIterable* CONST pThis)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIIterableGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return NULL;
	}
	else
	{
		return CSC_LinkedListFront(pLinkedList);
	}
}

static CSC_PVOID CSCMETHOD CSC_LinkedListIIterableNextElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CONST CSC_SIZE_T currentIndex, _In_ CONST CSC_PCVOID pCurrentElement)
{
	CSC_LLNode* pNode;
	CSC_LLNode* pCurrentNode = (CSC_LLNode*)NULL;
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIIterableGetObjectPointer(pThis);

	if (!pLinkedList || currentIndex == CSC_CONTAINER_INVALID_INDEX)
	{
		return NULL;
	}
	else
	{
		if (pCurrentElement)
		{
			pCurrentNode = (CSC_LLNode*)((CSC_BYTE* CONST)pCurrentElement - sizeof(CSC_LLNode));
		}

		pNode = CSC_LinkedListAccessNodeImpl(pLinkedList, currentIndex + (CSC_SIZE_T)1, currentIndex, pCurrentNode);

		return (pNode) ? CSC_LinkedListGetElementFromLLNode(pNode) : NULL;
	}
}

static CSC_PVOID CSCMETHOD CSC_LinkedListIIterableLastElement(_In_ CONST CSC_IIterable* CONST pThis)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIIterableGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return NULL;
	}
	else
	{
		return CSC_LinkedListBack(pLinkedList);
	}
}

static CSC_PVOID CSCMETHOD CSC_LinkedListIIterablePreviousElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CONST CSC_SIZE_T currentIndex, _In_ CONST CSC_PCVOID pCurrentElement)
{
	CSC_LLNode* pNode;
	CSC_LLNode* pCurrentNode = (CSC_LLNode*)NULL;
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIIterableGetObjectPointer(pThis);

	if (!pLinkedList || !currentIndex)
	{
		return NULL;
	}
	else
	{
		if (pCurrentElement)
		{
			pCurrentNode = (CSC_LLNode*)((CSC_BYTE* CONST)pCurrentElement - sizeof(CSC_LLNode));
		}

		pNode = CSC_LinkedListAccessNodeImpl(pLinkedList, currentIndex - (CSC_SIZE_T)1, currentIndex, pCurrentNode);

		return (pNode) ? CSC_LinkedListGetElementFromLLNode(pNode) : NULL;
	}
}

static CSC_PVOID CSCMETHOD CSC_LinkedListIIterableGetElementAt(_In_ CONST CSC_IIterable* CONST pThis, _In_ CONST CSC_SIZE_T index, _In_opt_ CONST CSC_SIZE_T currentIndex, _In_opt_ CONST CSC_PCVOID pCurrentElement)
{
	CSC_LLNode* pNode;
	CSC_LLNode* pCurrentNode = (CSC_LLNode*)NULL;
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIIterableGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return NULL;
	}
	else
	{
		if (currentIndex != CSC_CONTAINER_INVALID_INDEX && pCurrentElement)
		{
			pCurrentNode = (CSC_LLNode*)((CSC_BYTE* CONST)pCurrentElement - sizeof(CSC_LLNode));
		}

		pNode = CSC_LinkedListAccessNodeImpl(pLinkedList, index, currentIndex, pCurrentNode);

		return (pNode) ? CSC_LinkedListGetElementFromLLNode(pNode) : NULL;
	}
}

static CSC_SIZE_T CSCMETHOD CSC_LinkedListIIterableGetElementCount(_In_ CONST CSC_IIterable* CONST pThis)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIIterableGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return CSC_CONTAINER_INVALID_LENGTH;
	}
	else
	{
		return CSC_LinkedListGetSize(pLinkedList);
	}
}

static CSC_SIZE_T CSCMETHOD CSC_LinkedListIIterableGetElementSize(_In_ CONST CSC_IIterable* CONST pThis)
{
	CONST CSC_LinkedList* CONST pLinkedList = (CONST CSC_LinkedList* CONST)CSC_LinkedListIIterableGetObjectPointer(pThis);

	if (!pLinkedList)
	{
		return (CSC_SIZE_T)0;
	}
	else
	{
		return CSC_LinkedListGetElementSize(pLinkedList);
	}
}
