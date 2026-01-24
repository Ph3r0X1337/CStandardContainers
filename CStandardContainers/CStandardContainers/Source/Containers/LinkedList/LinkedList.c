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

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerInitialize(_Out_ CONST CSC_PVOID pMemoryBaseAddress, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_IAllocator* CONST pIAllocator);
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
static CSC_PVOID CSCMETHOD CSC_LinkedListIIterableNextElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement);
static CSC_PVOID CSCMETHOD CSC_LinkedListIIterableLastElement(_In_ CONST CSC_IIterable* CONST pThis);
static CSC_PVOID CSCMETHOD CSC_LinkedListIIterablePreviousElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement);
static CSC_PVOID CSCMETHOD CSC_LinkedListIIterableGetElementAt(_In_ CONST CSC_IIterable* CONST pThis, _In_ CONST CSC_SIZE_T index, _In_opt_ CONST CSC_SIZE_T currentIndex, _In_opt_ CONST CSC_PVOID pCurrentElement);
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


static CSC_STATUS CSCMETHOD CSC_LinkedListFillImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue);
static CSC_STATUS CSCMETHOD CSC_LinkedListInsertRangeImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements);
static CSC_STATUS CSCMETHOD CSC_LinkedListInsertListCopyImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_LinkedList* CONST pOther);
static CSC_STATUS CSCMETHOD CSC_LinkedListRemoveRangeImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements);
static CSC_LLNode* CSCMETHOD CSC_LinkedListAccessNodeImpl(_In_ CONST CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T index, _In_ CONST CSC_SIZE_T currentIndex, _In_opt_ CONST CSC_LLNode* CONST pCurrentNode);


static CSC_PVOID CSC_LinkedListGetElementFromLLNode(_In_ CONST CSC_LLNode* CONST pNode)
{
	return (CSC_PVOID)(pNode + (CSC_SIZE_T)1);
}


CSC_STATUS CSCMETHOD CSC_LinkedListInitialize(_Out_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_BOOLEAN circular, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	CONST CSC_STATUS status = CSC_LinkedListZeroMemory(pThis);

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

CSC_STATUS CSCMETHOD CSC_LinkedListInitializeWithSize(_Out_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_BOOLEAN circular, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	CSC_LinkedList llBuffer;
	CSC_STATUS status = CSC_LinkedListZeroMemory(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!elementSize || elementSize > CSC_LINKED_LIST_MAXIMUM_ELEMENT_SIZE || numOfElements > CSC_LINKED_LIST_MAXIMUM_SPACE / (elementSize + sizeof(CSC_LLNode)) || !pIAllocator || (numOfElements && pNestedContainerVTable && !pDefaultValue))
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	status = CSC_LinkedListInitialize(&llBuffer, elementSize, circular, pIAllocator, pNestedContainerVTable);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!numOfElements)
	{
		return CSC_STATUS_SUCCESS;
	}

	status = CSC_LinkedListFillImpl(&llBuffer, numOfElements, pDefaultValue);

	if (status != CSC_STATUS_SUCCESS)
	{
		CSC_LinkedListDestroy(&llBuffer);
		return status;
	}

	status = CSC_MemoryUtilsCopyMemory((CSC_PVOID)pThis, (CSC_PCVOID)&llBuffer, sizeof(llBuffer));

	if (status != CSC_STATUS_SUCCESS)
	{
		CSC_LinkedListDestroy(&llBuffer);
		return status;
	}

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListInitializeWithCopy(_Out_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_LinkedList* CONST pSrc)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListInitializeWithArray(_Out_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_BOOLEAN circular, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_PCVOID pElements, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	return CSC_STATUS_SUCCESS;
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

CSC_STATUS CSCMETHOD CSC_LinkedListZeroMemory(_Out_ CSC_LinkedList* CONST pThis)
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
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListMove(_Inout_ CSC_LinkedList* CONST pThis, _Inout_ CSC_LinkedList* CONST pSrc)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_LinkedListFill(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCMETHOD CSC_LinkedListFillImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListFillRange(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue)
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

static CSC_STATUS CSCMETHOD CSC_LinkedListInsertRangeImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_LinkedListInsertListCopy(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_LinkedList* CONST pOther)
{
	return CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCMETHOD CSC_LinkedListInsertListCopyImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_LinkedList* CONST pOther)
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

static CSC_STATUS CSCMETHOD CSC_LinkedListRemoveRangeImpl(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements)
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

static CSC_LLNode* CSCMETHOD CSC_LinkedListAccessNodeImpl(_In_ CONST CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T index, _In_ CONST CSC_SIZE_T currentIndex, _In_opt_ CONST CSC_LLNode* CONST pCurrentNode)
{
	return (CSC_LLNode*)NULL;
}


CSC_STATUS CSCMETHOD CSC_LinkedListReverse(_Inout_ CSC_LinkedList* CONST pThis)
{
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
		return ((pThis->circular) ? CSC_STATUS_GENERAL_FAILURE : CSC_STATUS_SUCCESS);
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

static CSC_STATUS CSCMETHOD CSC_LinkedListIContainerInitialize(_Out_ CONST CSC_PVOID pMemoryBaseAddress, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_IAllocator* CONST pIAllocator)
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

static CSC_PVOID CSCMETHOD CSC_LinkedListIIterableNextElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement)
{
	return NULL;
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

static CSC_PVOID CSCMETHOD CSC_LinkedListIIterablePreviousElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement)
{
	return NULL;
}

static CSC_PVOID CSCMETHOD CSC_LinkedListIIterableGetElementAt(_In_ CONST CSC_IIterable* CONST pThis, _In_ CONST CSC_SIZE_T index, _In_opt_ CONST CSC_SIZE_T currentIndex, _In_opt_ CONST CSC_PVOID pCurrentElement)
{
	return NULL;
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
