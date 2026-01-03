#include "DynamicArray.h"
#include "../../Interfaces/IBaseInterface.h"
#include "../../Utils/MemoryUtils.h"
#include <stddef.h>

// Definition of container limits and behaviour based on the data bus address width.
// Architectures with an address width of 32 bits or more share the same restrictions in maximum size.
#if CSC_ADDRESS_BUS_WIDTH_DATA < CSC_ADDRESS_BUS_WIDTH_DATA_32BIT
#define CSC_DYNAMIC_ARRAY_MINIMUM_SPACE (CSC_SIZE_T)0x10
#define CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE (CSC_SIZE_T)0x4000
#define CSC_DYNAMIC_ARRAY_LINEAR_ALLOCATION_THRESHOLD (CSC_SIZE_T)0x100
#define CSC_DYNAMIC_ARRAY_CALCULATION_ERROR (CSC_SIZE_T)-1
#else
#define CSC_DYNAMIC_ARRAY_MINIMUM_SPACE (CSC_SIZE_T)0x10
#define CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE (CSC_SIZE_T)0x40000000
#define CSC_DYNAMIC_ARRAY_LINEAR_ALLOCATION_THRESHOLD (CSC_SIZE_T)0x10000
#define CSC_DYNAMIC_ARRAY_CALCULATION_ERROR (CSC_SIZE_T)-1
#endif

typedef struct _CSC_DynamicArrayVirtualTable
{
	CSC_IBaseInterfaceVirtualTable baseInterfaceVTable;
	CSC_IContainerVirtualTable containerInterfaceVTable;
	CSC_IIterableVirtualTable iterableInterfaceVTable;
} CSC_DynamicArrayVirtualTable;

static CSC_PCVOID CSCMETHOD CSC_DynamicArrayIBaseInterfaceGetInterface(_In_ CONST CSC_IBaseInterface* CONST pThis, _In_ CONST EBaseInterfaceType interfaceType);

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerInitialize(_Inout_ CONST CSC_PVOID pMemoryBaseAddress, _In_ CONST CSC_SIZE_T elementSize, _In_ CSC_IAllocator* CONST pIAllocator);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerErase(_Inout_ CSC_IContainer* CONST pThis);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerDestroy(_Inout_ CSC_IContainer* CONST pThis);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerCopy(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST struct _CSC_IContainer* CONST pOther);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerMove(_Inout_ CSC_IContainer* CONST pThis, _Inout_ struct _CSC_IContainer* CONST pOther);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerInsertRange(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerRemoveRange(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerSwapValues(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T secondIndex);
static CSC_PVOID CSCMETHOD CSC_DynamicArrayIContainerAccessElement(_In_ CONST CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T index);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerIsValid(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerIsEmpty(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerIsElementContainer(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_SIZE_T CSCMETHOD CSC_DynamicArrayIContainerGetSize(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_SIZE_T CSCMETHOD CSC_DynamicArrayIContainerGetElementSize(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_SIZE_T CSCMETHOD CSC_DynamicArrayIContainerGetMaxElements(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_IAllocator* CSCMETHOD CSC_DynamicArrayIContainerGetIAllocator(_In_ CONST CSC_IContainer* CONST pThis);
static CSC_IContainerVirtualTable* CSCMETHOD CSC_DynamicArrayIContainerGetNestedContainerVTable(_In_ CONST struct _CSC_IContainer* CONST pThis);

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIIterableRegisterIterator(_Inout_ CSC_IIterable* CONST pThis, _In_ CONST CSC_IIterator* CONST pIIterator);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayIIterableUnregisterIterator(_Inout_ CSC_IIterable* CONST pThis);
static CSC_PVOID CSCMETHOD CSC_DynamicArrayIIterableFirstElement(_In_ CONST CSC_IIterable* CONST pThis);
static CSC_PVOID CSCMETHOD CSC_DynamicArrayIIterableNextElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement);
static CSC_PVOID CSCMETHOD CSC_DynamicArrayIIterableLastElement(_In_ CONST CSC_IIterable* CONST pThis);
static CSC_PVOID CSCMETHOD CSC_DynamicArrayIIterablePreviousElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement);
static CSC_PVOID CSCMETHOD CSC_DynamicArrayIIterableGetElementAt(_In_ CONST CSC_IIterable* CONST pThis, _In_ CONST CSC_SIZE_T index, _In_opt_ CONST CSC_SIZE_T currentIndex, _In_opt_ CONST CSC_PVOID pCurrentElement);
static CSC_SIZE_T CSCMETHOD CSC_DynamicArrayIIterableGetElementCount(_In_ CONST CSC_IIterable* CONST pThis);
static CSC_SIZE_T CSCMETHOD CSC_DynamicArrayIIterableGetElementSize(_In_ CONST CSC_IIterable* CONST pThis);

static CONST CSC_DynamicArrayVirtualTable g_dynamicArrayVirtualTable =
{
	{
		CSC_DynamicArrayIBaseInterfaceGetInterface
	},
	{
		CSC_DynamicArrayIContainerInitialize,
		CSC_DynamicArrayIContainerErase,
		CSC_DynamicArrayIContainerDestroy,
		CSC_DynamicArrayIContainerCopy,
		CSC_DynamicArrayIContainerMove,
		CSC_DynamicArrayIContainerInsertRange,
		CSC_DynamicArrayIContainerRemoveRange,
		CSC_DynamicArrayIContainerSwapValues,
		CSC_DynamicArrayIContainerAccessElement,
		CSC_DynamicArrayIContainerIsValid,
		CSC_DynamicArrayIContainerIsEmpty,
		CSC_DynamicArrayIContainerIsElementContainer,
		CSC_DynamicArrayIContainerGetSize,
		CSC_DynamicArrayIContainerGetElementSize,
		CSC_DynamicArrayIContainerGetMaxElements,
		CSC_DynamicArrayIContainerGetIAllocator,
		CSC_DynamicArrayIContainerGetNestedContainerVTable
	},
	{
		CSC_DynamicArrayIIterableRegisterIterator,
		CSC_DynamicArrayIIterableUnregisterIterator,
		CSC_DynamicArrayIIterableFirstElement,
		CSC_DynamicArrayIIterableNextElement,
		CSC_DynamicArrayIIterableLastElement,
		CSC_DynamicArrayIIterablePreviousElement,
		CSC_DynamicArrayIIterableGetElementAt,
		CSC_DynamicArrayIIterableGetElementCount,
		CSC_DynamicArrayIIterableGetElementSize
	}
};


static CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertRangeImpl(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayFillImpl(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayResizeImpl(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue, _In_ CONST CSC_BOOLEAN reserve, _In_ CONST CSC_BOOLEAN shrink);
static CSC_STATUS CSCMETHOD CSC_DynamicArrayRemoveRangeImpl(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_BOOLEAN shrink);


static CSC_SIZE_T CSCAPI CSC_DynamicArrayCalculateAllocReserve(_In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T numOfElements)
{
	CONST CSC_SIZE_T requestedSize = elementSize * numOfElements;
	CSC_SIZE_T calculatedSize = (CSC_DYNAMIC_ARRAY_MINIMUM_SPACE >> (CSC_SIZE_T)1);

	if (!numOfElements || !elementSize)
	{
		return CSC_DYNAMIC_ARRAY_CALCULATION_ERROR;
	}

	if (numOfElements > CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE / elementSize || elementSize > CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE)
	{
		return CSC_DYNAMIC_ARRAY_CALCULATION_ERROR;
	}

	if (requestedSize >= CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE)
	{
		return CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE;
	}

	if (requestedSize < (CSC_DYNAMIC_ARRAY_MINIMUM_SPACE >> (CSC_SIZE_T)1))
	{
		return CSC_DYNAMIC_ARRAY_MINIMUM_SPACE;
	}

	if (requestedSize >= CSC_DYNAMIC_ARRAY_LINEAR_ALLOCATION_THRESHOLD)
	{
		return (requestedSize & (~(CSC_DYNAMIC_ARRAY_LINEAR_ALLOCATION_THRESHOLD - (CSC_SIZE_T)1))) + ((CSC_SIZE_T)2 * CSC_DYNAMIC_ARRAY_LINEAR_ALLOCATION_THRESHOLD);
	}
	else
	{
		while (calculatedSize < requestedSize)
		{
			calculatedSize = calculatedSize << (CSC_SIZE_T)1;
		}

		return calculatedSize << (CSC_SIZE_T)1;
	}
}

static CSC_SIZE_T CSCAPI CSC_DynamicArrayCalculateRequestResize(_In_ CONST CSC_DynamicArray* CONST pDynamicArray, _In_ CONST CSC_SIZE_T numOfReserveElements, _In_ CONST CSC_BOOLEAN reserve)
{
	CSC_SIZE_T requestedSize, calculatedReserve;

	if (CSC_DynamicArrayIsValid(pDynamicArray) != CSC_STATUS_SUCCESS || numOfReserveElements > CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE / pDynamicArray->elementSize)
	{
		return CSC_DYNAMIC_ARRAY_CALCULATION_ERROR;
	}

	if ((reserve) ? (numOfReserveElements <= pDynamicArray->elementCount) : (numOfReserveElements == pDynamicArray->elementCount))
	{
		return pDynamicArray->reservedSpace;
	}

	requestedSize = pDynamicArray->elementSize * numOfReserveElements;

	if (requestedSize >= CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE)
	{
		return CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE;
	}

	if (requestedSize < (CSC_DYNAMIC_ARRAY_MINIMUM_SPACE >> (CSC_SIZE_T)1))
	{
		return CSC_DYNAMIC_ARRAY_MINIMUM_SPACE;
	}

	if (requestedSize >= CSC_DYNAMIC_ARRAY_LINEAR_ALLOCATION_THRESHOLD)
	{
		return (requestedSize & (~(CSC_DYNAMIC_ARRAY_LINEAR_ALLOCATION_THRESHOLD - (CSC_SIZE_T)1))) + CSC_DYNAMIC_ARRAY_LINEAR_ALLOCATION_THRESHOLD;
	}
	else
	{
		calculatedReserve = CSC_DYNAMIC_ARRAY_MINIMUM_SPACE;

		while (calculatedReserve < requestedSize)
		{
			calculatedReserve = calculatedReserve << (CSC_SIZE_T)1;
		}

		return calculatedReserve;
	}
}

static CSC_SIZE_T CSCAPI CSC_DynamicArrayCalculatePushSize(_In_ CONST CSC_DynamicArray* CONST pDynamicArray)
{
	CSC_SIZE_T requestedSize;

	if (CSC_DynamicArrayIsValid(pDynamicArray) != CSC_STATUS_SUCCESS)
	{
		return CSC_DYNAMIC_ARRAY_CALCULATION_ERROR;
	}

	requestedSize = pDynamicArray->elementSize * (pDynamicArray->elementCount + (CSC_SIZE_T)1);

	if (requestedSize <= pDynamicArray->reservedSpace)
	{
		return pDynamicArray->reservedSpace;
	}

	return CSC_DynamicArrayCalculateRequestResize(pDynamicArray, pDynamicArray->elementCount + (CSC_SIZE_T)1, (CSC_BOOLEAN)TRUE);
}

static CSC_SIZE_T CSCAPI CSC_DynamicArrayCalculatePopSize(_In_ CONST CSC_DynamicArray* CONST pDynamicArray)
{
	CSC_SIZE_T requestedSize;

	if (CSC_DynamicArrayIsValid(pDynamicArray) != CSC_STATUS_SUCCESS || !pDynamicArray->elementCount || !pDynamicArray->reservedSpace || !pDynamicArray->pData)
	{
		return CSC_DYNAMIC_ARRAY_CALCULATION_ERROR;
	}

	requestedSize = pDynamicArray->elementSize * (pDynamicArray->elementCount - (CSC_SIZE_T)1);

	if (requestedSize >= pDynamicArray->reservedSpace)
	{
		return CSC_DYNAMIC_ARRAY_CALCULATION_ERROR;
	}

	if (!requestedSize)
	{
		return requestedSize;
	}

	if (requestedSize >= CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE)
	{
		return CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE;
	}

	if (requestedSize < (CSC_DYNAMIC_ARRAY_MINIMUM_SPACE >> (CSC_SIZE_T)2))
	{
		return CSC_DYNAMIC_ARRAY_MINIMUM_SPACE;
	}

	if (pDynamicArray->reservedSpace > (CSC_DYNAMIC_ARRAY_LINEAR_ALLOCATION_THRESHOLD << (CSC_SIZE_T)1))
	{
		if (pDynamicArray->reservedSpace - requestedSize > CSC_DYNAMIC_ARRAY_LINEAR_ALLOCATION_THRESHOLD)
		{
			return pDynamicArray->reservedSpace - CSC_DYNAMIC_ARRAY_LINEAR_ALLOCATION_THRESHOLD;
		}
		else
		{
			return pDynamicArray->reservedSpace;
		}
	}
	else
	{
		if (requestedSize < (pDynamicArray->reservedSpace >> (CSC_SIZE_T)2))
		{
			return pDynamicArray->reservedSpace >> (CSC_SIZE_T)1;
		}
		else
		{
			return pDynamicArray->reservedSpace;
		}
	}
}


CSC_STATUS CSCMETHOD CSC_DynamicArrayInitialize(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	CONST CSC_STATUS status = CSC_DynamicArrayZeroMemory(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!elementSize || elementSize > CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE || !CSC_IAllocatorIsUsable(pIAllocator))
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	pThis->baseInterface.pIBaseInterfaceVirtualTable = &g_dynamicArrayVirtualTable.baseInterfaceVTable;
	pThis->containerInterface.pIContainerVirtualTable = &g_dynamicArrayVirtualTable.containerInterfaceVTable;
	pThis->iterableInterface.pIIterableVirtualTable = &g_dynamicArrayVirtualTable.iterableInterfaceVTable;

	pThis->elementSize = elementSize;
	pThis->elementCount = (CSC_SIZE_T)0;
	pThis->reservedSpace = (CSC_SIZE_T)0;
	pThis->pData = NULL;

	pThis->pIIterator = (CSC_IIterator*)NULL;
	pThis->pIAllocator = pIAllocator;

	pThis->pNestedContainerVTable = pNestedContainerVTable;

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithSize(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	CSC_DynamicArray arrayBuffer;
	CSC_STATUS status = CSC_DynamicArrayZeroMemory(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!elementSize || elementSize > CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE || numOfElements > CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE / elementSize || !pIAllocator || (numOfElements && pNestedContainerVTable && !pDefaultValue))
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	status = CSC_DynamicArrayInitialize(&arrayBuffer, elementSize, pIAllocator, pNestedContainerVTable);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!numOfElements)
	{
		return CSC_STATUS_SUCCESS;
	}

	status = CSC_DynamicArrayFillImpl(&arrayBuffer, numOfElements, pDefaultValue);

	if (status != CSC_STATUS_SUCCESS)
	{
		CSC_DynamicArrayDestroy(&arrayBuffer);
		return status;
	}

	status = CSC_MemoryUtilsCopyMemory((CSC_PVOID)pThis, (CSC_PCVOID)&arrayBuffer, sizeof(arrayBuffer));

	if (status != CSC_STATUS_SUCCESS)
	{
		CSC_DynamicArrayDestroy(&arrayBuffer);
		return status;
	}

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithCopy(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_DynamicArray* CONST pSrc)
{
	CSC_STATUS status;

	if (CSC_DynamicArrayIsValid(pSrc) != CSC_STATUS_SUCCESS)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pSrc->elementCount)
	{
		status = CSC_DynamicArrayInitializeWithArray(pThis, pSrc->elementSize, pSrc->elementCount, (CSC_PCVOID)pSrc->pData, pSrc->pIAllocator, pSrc->pNestedContainerVTable);
	}
	else
	{
		status = CSC_DynamicArrayInitialize(pThis, pSrc->elementSize, pSrc->pIAllocator, pSrc->pNestedContainerVTable);
	}

	return status;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithArray(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_PCVOID pElements, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	CSC_DynamicArray arrayBuffer;
	CSC_STATUS status = CSC_DynamicArrayZeroMemory(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!elementSize || elementSize > CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE || numOfElements > CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE / elementSize || !pIAllocator || !numOfElements || !pElements)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	status = CSC_DynamicArrayInitialize(&arrayBuffer, elementSize, pIAllocator, pNestedContainerVTable);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	status = CSC_DynamicArrayInsertRangeImpl(&arrayBuffer, (CSC_SIZE_T)0, numOfElements, pElements);

	if (status != CSC_STATUS_SUCCESS)
	{
		CSC_DynamicArrayDestroy(&arrayBuffer);
		return status;
	}

	status = CSC_MemoryUtilsCopyMemory((CSC_PVOID)pThis, (CSC_PCVOID)&arrayBuffer, sizeof(arrayBuffer));

	if (status != CSC_STATUS_SUCCESS)
	{
		CSC_DynamicArrayDestroy(&arrayBuffer);
		return status;
	}

	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_DynamicArrayResize(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue)
{
	return CSC_DynamicArrayResizeImpl(pThis, numOfElements, pDefaultValue, (CSC_BOOLEAN)FALSE, (CSC_BOOLEAN)TRUE);
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyResize(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue)
{
	return CSC_DynamicArrayResizeImpl(pThis, numOfElements, pDefaultValue, (CSC_BOOLEAN)FALSE, (CSC_BOOLEAN)FALSE);
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayReserve(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements)
{
	return CSC_DynamicArrayResizeImpl(pThis, numOfElements, (CSC_PCVOID)NULL, (CSC_BOOLEAN)TRUE, (CSC_BOOLEAN)FALSE);
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayShrinkToFit(_Inout_ CSC_DynamicArray* CONST pThis)
{
	if (!pThis)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	return CSC_DynamicArrayResizeImpl(pThis, pThis->elementCount, (CSC_PCVOID)NULL, (CSC_BOOLEAN)TRUE, (CSC_BOOLEAN)TRUE);
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayResizeImpl(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue, _In_ CONST CSC_BOOLEAN reserve, _In_ CONST CSC_BOOLEAN shrink)
{
	CONST CSC_IContainer* pDefaultValueIContainer;
	CSC_IContainer* pIteratorIContainer;
	CSC_IContainer* pIteratorIContainerSrc;
	CSC_PVOID pNewData;
	CSC_SIZE_T allocationSize, iterator, oldSize;
	CSC_STATUS status = CSC_DynamicArrayIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!reserve && numOfElements > pThis->elementCount && pThis->pNestedContainerVTable && !pDefaultValue)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	allocationSize = CSC_DynamicArrayCalculateRequestResize(pThis, numOfElements, reserve);

	if (allocationSize == CSC_DYNAMIC_ARRAY_CALCULATION_ERROR)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pThis->pNestedContainerVTable)
	{
		pDefaultValueIContainer = (CONST CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)pDefaultValue, csc_bit_IContainer);

		if (!pDefaultValueIContainer || pDefaultValueIContainer->pIContainerVirtualTable != pThis->pNestedContainerVTable)
		{
			return CSC_STATUS_INVALID_PARAMETER;
		}
	}

	if (reserve)
	{
		if ((shrink) ? (allocationSize >= pThis->reservedSpace) : (allocationSize <= pThis->reservedSpace))
		{
			return CSC_STATUS_SUCCESS;
		}
	}
	else
	{
		if ((shrink) ? (allocationSize == pThis->reservedSpace) : (allocationSize <= pThis->reservedSpace))
		{
			if (numOfElements > pThis->elementCount)
			{
				if (pThis->pNestedContainerVTable)
				{
					for (iterator = pThis->elementCount; iterator < numOfElements; ++iterator)
					{
						status = pThis->pNestedContainerVTable->pInitialize((CSC_PVOID)((CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), pThis->pNestedContainerVTable->pGetElementSize(pDefaultValueIContainer), pThis->pIAllocator);

						if (status != CSC_STATUS_SUCCESS)
						{
							--iterator;
							break;
						}

						pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
						status = pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, pDefaultValueIContainer);

						if (status != CSC_STATUS_SUCCESS)
						{
							break;
						}
					}

					if (status != CSC_STATUS_SUCCESS)
					{
						while (iterator >= pThis->elementCount && iterator < numOfElements)
						{
							pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
							pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);

							if (iterator == pThis->elementCount)
							{
								break;
							}

							--iterator;
						}

						return status;
					}
				}
				else
				{
					if (pDefaultValue)
					{
						status = CSC_MemoryUtilsSetArrayValue((CSC_PVOID)((CSC_BYTE* CONST)pThis->pData + pThis->elementSize * pThis->elementCount), pDefaultValue, pThis->elementSize, numOfElements - pThis->elementCount);
					}
					else
					{
						status = CSC_MemoryUtilsSetZeroMemory((CSC_PVOID)((CSC_BYTE* CONST)pThis->pData + pThis->elementSize * pThis->elementCount), pThis->elementSize * (numOfElements - pThis->elementCount));
					}

					if (status != CSC_STATUS_SUCCESS)
					{
						return status;
					}
				}

				oldSize = pThis->elementCount;
				pThis->elementCount = numOfElements;

				if (pThis->pIIterator)
				{
					CSC_IIteratorOnInsertion(pThis->pIIterator, oldSize, numOfElements - oldSize, numOfElements);
				}
			}
			else if (numOfElements < pThis->elementCount)
			{
				if (pThis->pNestedContainerVTable)
				{
					for (iterator = numOfElements; iterator < pThis->elementCount; ++iterator)
					{
						pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
						pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
					}
				}

				oldSize = pThis->elementCount;
				pThis->elementCount = numOfElements;

				if (pThis->pIIterator)
				{
					CSC_IIteratorOnRemoval(pThis->pIIterator, numOfElements, oldSize - numOfElements, numOfElements);
				}
			}

			return CSC_STATUS_SUCCESS;
		}
	}

	if (!CSC_IAllocatorIsUsable(pThis->pIAllocator))
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}

	if (!allocationSize || (!reserve && !numOfElements))
	{
		if (pThis->pData)
		{
			if (pThis->pNestedContainerVTable)
			{
				for (iterator = (CSC_SIZE_T)0; iterator < pThis->elementCount; ++iterator)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
				}
			}

			CSC_IAllocatorFree(pThis->pIAllocator, pThis->pData);
		}

		return status;
	}

	pNewData = CSC_IAllocatorAllocZero(pThis->pIAllocator, allocationSize);

	if (!pNewData)
	{
		return CSC_STATUS_MEMORY_NOT_ALLOCATED;
	}

	if (pThis->pData)
	{
		if (reserve)
		{
			if (pThis->pNestedContainerVTable)
			{
				for (iterator = (CSC_SIZE_T)0; iterator < pThis->elementCount; ++iterator)
				{
					pIteratorIContainerSrc = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
					status = pThis->pNestedContainerVTable->pInitialize((CSC_PVOID)((CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), pThis->pNestedContainerVTable->pGetElementSize(pIteratorIContainerSrc), pThis->pIAllocator);

					if (status != CSC_STATUS_SUCCESS)
					{
						--iterator;
						break;
					}

					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
					status = pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, (CONST CSC_IContainer*)pIteratorIContainerSrc);

					if (status != CSC_STATUS_SUCCESS)
					{
						break;
					}
				}

				if (status != CSC_STATUS_SUCCESS)
				{
					while (iterator >= (CSC_SIZE_T)0 && iterator < pThis->elementCount)
					{
						pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
						pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);

						if (!iterator)
						{
							break;
						}

						--iterator;
					}
				}
				else
				{
					for (iterator = (CSC_SIZE_T)0; iterator < pThis->elementCount; ++iterator)
					{
						pIteratorIContainerSrc = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
						pThis->pNestedContainerVTable->pDestroy(pIteratorIContainerSrc);
					}
				}
			}
			else
			{
				status = CSC_MemoryUtilsCopyMemory(pNewData, (CSC_PCVOID)pThis->pData, pThis->elementSize * pThis->elementCount);
			}
		}
		else
		{
			if (numOfElements > pThis->elementCount)
			{
				if (pThis->pNestedContainerVTable)
				{
					for (iterator = (CSC_SIZE_T)0; iterator < pThis->elementCount; ++iterator)
					{
						pIteratorIContainerSrc = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
						status = pThis->pNestedContainerVTable->pInitialize((CSC_PVOID)((CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), pThis->pNestedContainerVTable->pGetElementSize(pIteratorIContainerSrc), pThis->pIAllocator);

						if (status != CSC_STATUS_SUCCESS)
						{
							--iterator;
							break;
						}

						pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
						status = pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, (CONST CSC_IContainer*)pIteratorIContainerSrc);

						if (status != CSC_STATUS_SUCCESS)
						{
							break;
						}
					}

					if (status != CSC_STATUS_SUCCESS)
					{
						while (iterator >= (CSC_SIZE_T)0 && iterator < pThis->elementCount)
						{
							pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
							pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);

							if (!iterator)
							{
								break;
							}

							--iterator;
						}
					}
					else
					{
						for (iterator = pThis->elementCount; iterator < numOfElements; ++iterator)
						{
							status = pThis->pNestedContainerVTable->pInitialize((CSC_PVOID)((CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), pThis->pNestedContainerVTable->pGetElementSize(pDefaultValueIContainer), pThis->pIAllocator);

							if (status != CSC_STATUS_SUCCESS)
							{
								--iterator;
								break;
							}

							pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
							status = pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, pDefaultValueIContainer);

							if (status != CSC_STATUS_SUCCESS)
							{
								break;
							}
						}

						if (status != CSC_STATUS_SUCCESS)
						{
							while (iterator >= (CSC_SIZE_T)0 && iterator < numOfElements)
							{
								pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
								pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);

								if (!iterator)
								{
									break;
								}

								--iterator;
							}
						}
						else
						{
							for (iterator = (CSC_SIZE_T)0; iterator < pThis->elementCount; ++iterator)
							{
								pIteratorIContainerSrc = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
								pThis->pNestedContainerVTable->pDestroy(pIteratorIContainerSrc);
							}
						}
					}
				}
				else
				{
					status = CSC_MemoryUtilsCopyMemory(pNewData, (CSC_PCVOID)pThis->pData, pThis->elementSize * pThis->elementCount);

					if (status == CSC_STATUS_SUCCESS && pDefaultValue)
					{
						status = CSC_MemoryUtilsSetArrayValue((CSC_PVOID)((CSC_BYTE* CONST)pNewData + pThis->elementSize * pThis->elementCount), pDefaultValue, pThis->elementSize, numOfElements - pThis->elementCount);
					}
				}
			}
			else if (numOfElements < pThis->elementCount)
			{
				if (pThis->pNestedContainerVTable)
				{
					for (iterator = (CSC_SIZE_T)0; iterator < numOfElements; ++iterator)
					{
						pIteratorIContainerSrc = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
						status = pThis->pNestedContainerVTable->pInitialize((CSC_PVOID)((CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), pThis->pNestedContainerVTable->pGetElementSize(pIteratorIContainerSrc), pThis->pIAllocator);

						if (status != CSC_STATUS_SUCCESS)
						{
							--iterator;
							break;
						}

						pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
						status = pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, (CONST CSC_IContainer*)pIteratorIContainerSrc);

						if (status != CSC_STATUS_SUCCESS)
						{
							break;
						}
					}

					if (status != CSC_STATUS_SUCCESS)
					{
						while (iterator >= (CSC_SIZE_T)0 && iterator < numOfElements)
						{
							pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
							pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);

							if (!iterator)
							{
								break;
							}

							--iterator;
						}
					}
					else
					{
						for (iterator = (CSC_SIZE_T)0; iterator < pThis->elementCount; ++iterator)
						{
							pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
							pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
						}
					}
				}
				else
				{
					status = CSC_MemoryUtilsCopyMemory(pNewData, (CSC_PCVOID)pThis->pData, pThis->elementSize * numOfElements);
				}
			}
		}

		if (status != CSC_STATUS_SUCCESS)
		{
			CSC_IAllocatorFree(pThis->pIAllocator, pNewData);
			return status;
		}

		CSC_IAllocatorFree(pThis->pIAllocator, pThis->pData);
	}
	else if (!reserve && pThis->pNestedContainerVTable)
	{
		for (iterator = (CSC_SIZE_T)0; iterator < numOfElements; ++iterator)
		{
			status = pThis->pNestedContainerVTable->pInitialize((CSC_PVOID)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), pThis->pNestedContainerVTable->pGetElementSize(pDefaultValueIContainer), pThis->pIAllocator);

			if (status != CSC_STATUS_SUCCESS)
			{
				--iterator;
				break;
			}

			pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
			status = pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, pDefaultValueIContainer);

			if (status != CSC_STATUS_SUCCESS)
			{
				break;
			}
		}

		if (status != CSC_STATUS_SUCCESS)
		{
			while (iterator >= (CSC_SIZE_T)0 && iterator < numOfElements)
			{
				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
				pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);

				if (!iterator)
				{
					break;
				}

				--iterator;
			}

			CSC_IAllocatorFree(pThis->pIAllocator, pNewData);
			return status;
		}
	}

	pThis->pData = pNewData;
	pThis->reservedSpace = allocationSize;

	if (!reserve)
	{
		oldSize = pThis->elementCount;
		pThis->elementCount = numOfElements;

		if (pThis->pIIterator)
		{
			if (numOfElements > oldSize)
			{
				CSC_IIteratorOnInsertion(pThis->pIIterator, oldSize, numOfElements - oldSize, numOfElements);
			}
			else if (numOfElements < oldSize)
			{
				CSC_IIteratorOnRemoval(pThis->pIIterator, numOfElements, oldSize - numOfElements, numOfElements);
			}
			else
			{
				CSC_IIteratorUpdateIteration(pThis->pIIterator);
			}
		}
	}
	else if (pThis->pIIterator)
	{
		CSC_IIteratorUpdateIteration(pThis->pIIterator);
	}

	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_DynamicArrayDestroy(_Inout_ CSC_DynamicArray* CONST pThis)
{
	CSC_STATUS status = CSC_DynamicArrayErase(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (pThis->pIIterator)
	{
		CSC_IIteratorOnDestruction(pThis->pIIterator);
	}

	return CSC_DynamicArrayZeroMemory(pThis);
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayErase(_Inout_ CSC_DynamicArray* CONST pThis)
{
	CONST CSC_STATUS status = CSC_DynamicArrayClear(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (pThis->pData)
	{
		if (!CSC_IAllocatorIsUsable(pThis->pIAllocator))
		{
			return CSC_STATUS_GENERAL_FAILURE;
		}

		CSC_IAllocatorFree(pThis->pIAllocator, pThis->pData);
		pThis->pData = NULL;
	}

	pThis->reservedSpace = (CSC_SIZE_T)0;

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayClear(_Inout_ CSC_DynamicArray* CONST pThis)
{
	CSC_SIZE_T iterator;
	CSC_IContainer* pIteratorIContainer;
	CSC_STATUS status = CSC_DynamicArrayIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (pThis->elementCount && pThis->pNestedContainerVTable)
	{
		for (iterator = (CSC_SIZE_T)0; iterator < pThis->elementCount; ++iterator)
		{
			pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
			pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
		}
	}

	pThis->elementCount = (CSC_SIZE_T)0;

	if (pThis->pIIterator)
	{
		CSC_IIteratorInvalidateIteration(pThis->pIIterator);
	}

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayZeroMemory(_Out_ CSC_DynamicArray* CONST pThis)
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
	pThis->reservedSpace = (CSC_SIZE_T)0;
	pThis->pData = NULL;

	pThis->pIIterator = (CSC_IIterator*)NULL;
	pThis->pIAllocator = (CONST CSC_IAllocator*)NULL;

	pThis->pNestedContainerVTable = (CONST CSC_IContainerVirtualTable*)NULL;

	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_DynamicArrayPushValue(_Inout_ CSC_DynamicArray* CONST pThis, _In_opt_ CONST CSC_PCVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayPopValue(_Inout_ CSC_DynamicArray* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyPopValue(_Inout_ CSC_DynamicArray* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayPopFront(_Inout_ CSC_DynamicArray* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyPopFront(_Inout_ CSC_DynamicArray* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}


CSC_PVOID CSCMETHOD CSC_DynamicArrayAccessElement(_In_ CONST CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T index)
{
	return NULL;
}


CSC_STATUS CSCMETHOD CSC_DynamicArrayFill(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayFillImpl(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue)
{

}

CSC_STATUS CSCMETHOD CSC_DynamicArrayFillRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_DynamicArrayCopy(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_DynamicArray* CONST pSrc)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayMove(_Inout_ CSC_DynamicArray* CONST pThis, _Inout_ CSC_DynamicArray* CONST pSrc)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayCopyArray(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_PCVOID pElements)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertElement(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_opt_ CONST CSC_PCVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements)
{
	return CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertRangeImpl(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements)
{
	CSC_PVOID pNewData;
	CSC_SIZE_T allocationSize, iterator;
	CSC_IContainer* pIteratorIContainer;
	CSC_IContainer* pIteratorIContainerSrc;
	CONST CSC_IContainer* pElementIContainer;
	CSC_STATUS status = CSC_DynamicArrayIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (!numOfElements || insertIndex > pThis->elementCount || pThis->elementCount + numOfElements > CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE / pThis->elementSize || (pThis->pNestedContainerVTable && !pElements))
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (!pThis->elementCount)
	{
		allocationSize = CSC_DynamicArrayCalculateAllocReserve(pThis->elementSize, numOfElements);
	}
	else
	{
		allocationSize = CSC_DynamicArrayCalculateRequestResize(pThis, pThis->elementCount + numOfElements, (CSC_BOOLEAN)FALSE);
	}

	if (allocationSize == CSC_DYNAMIC_ARRAY_CALCULATION_ERROR)
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

	if (allocationSize <= pThis->reservedSpace)
	{
		if (pThis->pNestedContainerVTable)
		{
			for (iterator = pThis->elementCount; iterator < pThis->elementCount + numOfElements; ++iterator)
			{
				pElementIContainer = (CONST CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pElements + pThis->elementSize * (iterator - pThis->elementCount)), csc_bit_IContainer);
				status = pThis->pNestedContainerVTable->pInitialize((CSC_PVOID)((CSC_BYTE * CONST)pThis->pData + pThis->elementSize * iterator), pThis->pNestedContainerVTable->pGetElementSize(pElementIContainer), pThis->pIAllocator);

				if (status != CSC_STATUS_SUCCESS)
				{
					--iterator;
					break;
				}
			}

			if (status != CSC_STATUS_SUCCESS)
			{
				while (iterator >= pThis->elementCount && iterator < pThis->elementCount + numOfElements)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);

					if (iterator == pThis->elementCount)
					{
						break;
					}

					--iterator;
				}

				return status;
			}

			for (iterator = (CSC_SIZE_T)0; iterator < pThis->elementCount - insertIndex; ++iterator)
			{
				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * ((pThis->elementCount + numOfElements) - iterator - (CSC_SIZE_T)1)), csc_bit_IContainer);
				pIteratorIContainerSrc = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * (pThis->elementCount - iterator - (CSC_SIZE_T)1)), csc_bit_IContainer);
				status = pThis->pNestedContainerVTable->pMove(pIteratorIContainer, pIteratorIContainerSrc);

				if (status != CSC_STATUS_SUCCESS)
				{
					--iterator;
					break;
				}
			}

			if (status != CSC_STATUS_SUCCESS)
			{
				while (iterator >= (CSC_SIZE_T)0 && iterator < pThis->elementCount - insertIndex)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * (pThis->elementCount - iterator - (CSC_SIZE_T)1)), csc_bit_IContainer);
					pIteratorIContainerSrc = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * ((pThis->elementCount + numOfElements) - iterator - (CSC_SIZE_T)1)), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pMove(pIteratorIContainer, pIteratorIContainerSrc);

					if (!iterator)
					{
						break;
					}

					--iterator;
				}

				for (iterator = pThis->elementCount; iterator < pThis->elementCount + numOfElements; ++iterator)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
				}

				return status;
			}

			for (iterator = (CSC_SIZE_T)0; iterator < numOfElements; ++iterator)
			{
				pElementIContainer = (CONST CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pElements + pThis->elementSize * iterator), csc_bit_IContainer);
				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * (insertIndex + iterator)), csc_bit_IContainer);

				status = pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, pElementIContainer);

				if (status != CSC_STATUS_SUCCESS)
				{
					break;
				}
			}

			if (status != CSC_STATUS_SUCCESS)
			{
				for (iterator = (CSC_SIZE_T)0; iterator < numOfElements; ++iterator)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * (insertIndex + iterator)), csc_bit_IContainer);
					pIteratorIContainerSrc = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * (insertIndex + numOfElements + iterator)), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pMove(pIteratorIContainer, pIteratorIContainerSrc);
				}

				for (iterator = pThis->elementCount; iterator < pThis->elementCount + numOfElements; ++iterator)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
				}

				return status;
			}
		}
		else
		{
			if (insertIndex < pThis->elementCount)
			{
				status = CSC_MemoryUtilsCopyMemory((CSC_PVOID)((CSC_BYTE* CONST)pThis->pData + pThis->elementSize * (insertIndex + numOfElements)), (CSC_PCVOID)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * insertIndex), pThis->elementSize * (pThis->elementCount - insertIndex));

				if (status != CSC_STATUS_SUCCESS)
				{
					return status;
				}

				status = (pElements) ? CSC_MemoryUtilsCopyMemory((CSC_PVOID)((CSC_BYTE* CONST)pThis->pData + pThis->elementSize * insertIndex), pElements, pThis->elementSize * numOfElements) : CSC_MemoryUtilsSetZeroMemory((CSC_PVOID)((CSC_BYTE* CONST)pThis->pData + pThis->elementSize * insertIndex), pThis->elementSize * numOfElements);

				if (status != CSC_STATUS_SUCCESS)
				{
					CSC_MemoryUtilsCopyMemory((CSC_PVOID)((CSC_BYTE* CONST)pThis->pData + pThis->elementSize * insertIndex), (CSC_PCVOID)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * (insertIndex + numOfElements)), pThis->elementSize* (pThis->elementCount - insertIndex));
				}
			}
			else
			{
				status = (pElements) ? CSC_MemoryUtilsCopyMemory((CSC_PVOID)((CSC_BYTE* CONST)pThis->pData + pThis->elementSize * insertIndex), pElements, pThis->elementSize * numOfElements) : CSC_MemoryUtilsSetZeroMemory((CSC_PVOID)((CSC_BYTE* CONST)pThis->pData + pThis->elementSize * insertIndex), pThis->elementSize * numOfElements);
			}
		}

		if (status == CSC_STATUS_SUCCESS)
		{
			pThis->elementCount += numOfElements;

			if (pThis->pIIterator)
			{
				CSC_IIteratorOnInsertion(pThis->pIIterator, insertIndex, numOfElements, pThis->elementCount);
			}
		}

		return status;
	}

	if (!CSC_IAllocatorIsUsable(pThis->pIAllocator))
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}

	pNewData = CSC_IAllocatorAllocZero(pThis->pIAllocator, allocationSize);

	if (!pNewData)
	{
		return CSC_STATUS_MEMORY_NOT_ALLOCATED;
	}

	if (pThis->pData)
	{
		if (pThis->pNestedContainerVTable)
		{
			for (iterator = (CSC_SIZE_T)0; iterator < insertIndex; ++iterator)
			{
				pIteratorIContainerSrc = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
				status = pThis->pNestedContainerVTable->pInitialize((CSC_PVOID)((CSC_BYTE * CONST)pNewData + pThis->elementSize * iterator), pThis->pNestedContainerVTable->pGetElementSize(pIteratorIContainerSrc), pThis->pIAllocator);

				if (status != CSC_STATUS_SUCCESS)
				{
					--iterator;
					break;
				}

				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
				status = pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, (CONST CSC_IContainer*)pIteratorIContainerSrc);

				if (status != CSC_STATUS_SUCCESS)
				{
					break;
				}
			}

			if (status != CSC_STATUS_SUCCESS)
			{
				while (iterator >= (CSC_SIZE_T)0 && iterator < insertIndex)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface * CONST)((CONST CSC_BYTE * CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);

					if (!iterator)
					{
						break;
					}

					--iterator;
				}

				CSC_IAllocatorFree(pThis->pIAllocator, pNewData);
				return status;
			}

			for (iterator = (CSC_SIZE_T)0; iterator < numOfElements; ++iterator)
			{
				pElementIContainer = (CONST CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pElements + pThis->elementSize * iterator), csc_bit_IContainer);
				status = pThis->pNestedContainerVTable->pInitialize((CSC_PVOID)((CSC_BYTE* CONST)pNewData + pThis->elementSize * (insertIndex + iterator)), pThis->pNestedContainerVTable->pGetElementSize(pElementIContainer), pThis->pIAllocator);

				if (status != CSC_STATUS_SUCCESS)
				{
					if (!iterator)
					{
						iterator = CSC_CONTAINER_INVALID_INDEX;
					}
					else
					{
						--iterator;
					}
					break;
				}

				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * (insertIndex + iterator)), csc_bit_IContainer);
				status = pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, pElementIContainer);

				if (status != CSC_STATUS_SUCCESS)
				{
					break;
				}
			}

			if (status != CSC_STATUS_SUCCESS)
			{
				if (iterator == CSC_CONTAINER_INVALID_INDEX)
				{
					iterator = (insertIndex) ? insertIndex - (CSC_SIZE_T)1 : CSC_CONTAINER_INVALID_INDEX;
				}
				else
				{
					iterator += insertIndex;
				}
				
				while (iterator >= (CSC_SIZE_T)0 && iterator < insertIndex + numOfElements)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);

					if (!iterator)
					{
						break;
					}

					--iterator;
				}

				CSC_IAllocatorFree(pThis->pIAllocator, pNewData);
				return status;
			}

			if (insertIndex < pThis->elementCount)
			{
				for (iterator = insertIndex; iterator < pThis->elementCount; ++iterator)
				{
					pIteratorIContainerSrc = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
					status = pThis->pNestedContainerVTable->pInitialize((CSC_PVOID)((CSC_BYTE* CONST)pNewData + pThis->elementSize * (iterator + numOfElements)), pThis->pNestedContainerVTable->pGetElementSize(pIteratorIContainerSrc), pThis->pIAllocator);

					if (status != CSC_STATUS_SUCCESS)
					{
						if (!iterator)
						{
							iterator = CSC_CONTAINER_INVALID_INDEX;
						}
						else
						{
							--iterator;
						}
						break;
					}

					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * (iterator + numOfElements)), csc_bit_IContainer);
					status = pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, (CONST CSC_IContainer*)pIteratorIContainerSrc);

					if (status != CSC_STATUS_SUCCESS)
					{
						break;
					}
				}

				if (status != CSC_STATUS_SUCCESS)
				{
					if (iterator == CSC_CONTAINER_INVALID_INDEX)
					{
						iterator = insertIndex + numOfElements - (CSC_SIZE_T)1;
					}
					else
					{
						iterator += numOfElements;
					}

					while (iterator >= (CSC_SIZE_T)0 && iterator < pThis->elementCount + numOfElements)
					{
						pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
						pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);

						if (!iterator)
						{
							break;
						}

						--iterator;
					}

					CSC_IAllocatorFree(pThis->pIAllocator, pNewData);
					return status;
				}
			}
		}
		else
		{
			status = CSC_MemoryUtilsCopyMemory(pNewData, (CSC_PCVOID)pThis->pData, pThis->elementSize * insertIndex);

			if (status != CSC_STATUS_SUCCESS)
			{
				CSC_IAllocatorFree(pThis->pIAllocator, pNewData);
				return status;
			}

			if (insertIndex < pThis->elementCount)
			{
				status = CSC_MemoryUtilsCopyMemory((CSC_PVOID)((CSC_BYTE* CONST)pNewData + pThis->elementSize * (insertIndex + numOfElements)), (CSC_PCVOID)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * insertIndex), pThis->elementSize * (pThis->elementCount - insertIndex));

				if (status != CSC_STATUS_SUCCESS)
				{
					CSC_IAllocatorFree(pThis->pIAllocator, pNewData);
					return status;
				}
			}

			if (pElements)
			{
				status = CSC_MemoryUtilsCopyMemory((CSC_PVOID)((CSC_BYTE* CONST)pNewData + pThis->elementSize * insertIndex), pElements, pThis->elementSize * numOfElements);
			}
		}

		if (status != CSC_STATUS_SUCCESS)
		{
			CSC_IAllocatorFree(pThis->pIAllocator, pNewData);
			return status;
		}

		CSC_IAllocatorFree(pThis->pIAllocator, pThis->pData);
	}
	else if (pThis->pNestedContainerVTable)
	{
		for (iterator = (CSC_SIZE_T)0; iterator < numOfElements; ++iterator)
		{
			pElementIContainer = (CONST CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pElements + pThis->elementSize * iterator), csc_bit_IContainer);
			status = pThis->pNestedContainerVTable->pInitialize((CSC_PVOID)((CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), pThis->pNestedContainerVTable->pGetElementSize(pElementIContainer), pThis->pIAllocator);

			if (status != CSC_STATUS_SUCCESS)
			{
				--iterator;
				break;
			}

			pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
			status = pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, pElementIContainer);

			if (status != CSC_STATUS_SUCCESS)
			{
				break;
			}
		}

		while (iterator >= (CSC_SIZE_T)0 && iterator < numOfElements)
		{
			pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
			pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);

			if (!iterator)
			{
				break;
			}

			--iterator;
		}
	}

	pThis->pData = pNewData;
	pThis->reservedSpace = allocationSize;
	pThis->elementCount += numOfElements;

	if (pThis->pIIterator)
	{
		CSC_IIteratorOnInsertion(pThis->pIIterator, insertIndex, numOfElements, pThis->elementCount);
	}

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertArray(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_DynamicArray* CONST pSrc)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayAppendCopy(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_DynamicArray* CONST pSrc)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayAppendMove(_Inout_ CSC_DynamicArray* CONST pThis, _Inout_ CSC_DynamicArray* CONST pSrc)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_DynamicArrayRemoveElement(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyRemoveElement(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayRemoveRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements)
{
	return CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayRemoveRangeImpl(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_BOOLEAN shrink)
{
	CSC_PVOID pNewData;
	CSC_SIZE_T allocationSize, iterator;
	CSC_IContainer* pIteratorIContainer;
	CSC_IContainer* pIteratorIContainerSrc;
	CSC_STATUS status = CSC_DynamicArrayIsValid(pThis);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if (removeIndex + numOfElements > pThis->elementCount || !numOfElements)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (numOfElements == pThis->elementCount)
	{
		return CSC_DynamicArrayErase(pThis);
	}

	if (!shrink)
	{
		if (pThis->pNestedContainerVTable)
		{
			if (removeIndex + numOfElements < pThis->elementCount)
			{
				for (iterator = (CSC_SIZE_T)0; iterator < pThis->elementCount - (removeIndex + numOfElements); ++iterator)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * (removeIndex + iterator)), csc_bit_IContainer);
					pIteratorIContainerSrc = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * (removeIndex + numOfElements + iterator)), csc_bit_IContainer);

					status = pThis->pNestedContainerVTable->pMove(pIteratorIContainer, pIteratorIContainerSrc);

					if (status != CSC_STATUS_SUCCESS)
					{
						return status;
					}
				}
			}

			for (iterator = pThis->elementCount - numOfElements; iterator < pThis->elementCount; ++iterator)
			{
				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
				pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
			}
		}
		else
		{
			if (removeIndex + numOfElements < pThis->elementCount)
			{
				status = CSC_MemoryUtilsCopyMemory((CSC_PVOID)((CSC_BYTE* CONST)pThis->pData + pThis->elementSize * removeIndex), (CSC_PCVOID)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * (removeIndex + numOfElements)), pThis->elementSize * (pThis->elementCount - (removeIndex + numOfElements)));

				if (status != CSC_STATUS_SUCCESS)
				{
					return status;
				}
			}
		}

		pThis->elementCount -= numOfElements;

		if (pThis->pIIterator)
		{
			CSC_IIteratorOnRemoval(pThis->pIIterator, removeIndex, numOfElements, pThis->elementCount);
		}

		return CSC_STATUS_SUCCESS;
	}

	if (!CSC_IAllocatorIsUsable(pThis->pIAllocator))
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	allocationSize = CSC_DynamicArrayCalculateRequestResize(pThis, pThis->elementCount - numOfElements, (CSC_BOOLEAN)FALSE);

	if (allocationSize == CSC_DYNAMIC_ARRAY_CALCULATION_ERROR)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	pNewData = CSC_IAllocatorAllocZero(pThis->pIAllocator, allocationSize);

	if (!pNewData)
	{
		return CSC_STATUS_MEMORY_NOT_ALLOCATED;
	}

	if (pThis->pNestedContainerVTable)
	{
		if (removeIndex)
		{
			for (iterator = (CSC_SIZE_T)0; iterator < removeIndex; ++iterator)
			{
				pIteratorIContainerSrc = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
				status = pThis->pNestedContainerVTable->pInitialize((CSC_PVOID)((CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), pThis->pNestedContainerVTable->pGetElementSize(pIteratorIContainerSrc), pThis->pIAllocator);

				if (status != CSC_STATUS_SUCCESS)
				{
					--iterator;
					break;
				}

				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
				status = pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, (CONST CSC_IContainer*)pIteratorIContainerSrc);

				if (status != CSC_STATUS_SUCCESS)
				{
					break;
				}
			}

			if (status != CSC_STATUS_SUCCESS)
			{
				while (iterator >= (CSC_SIZE_T)0 && iterator < removeIndex)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);

					if (!iterator)
					{
						break;
					}

					--iterator;
				}

				CSC_IAllocatorFree(pThis->pIAllocator, pNewData);
				return status;
			}
		}

		if (removeIndex + numOfElements < pThis->elementCount)
		{
			for (iterator = (CSC_SIZE_T)0; iterator < pThis->elementCount - (removeIndex + numOfElements); ++iterator)
			{
				pIteratorIContainerSrc = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * (removeIndex + numOfElements + iterator)), csc_bit_IContainer);
				status = pThis->pNestedContainerVTable->pInitialize((CSC_PVOID)((CSC_BYTE* CONST)pNewData + pThis->elementSize * (removeIndex + iterator)), pThis->pNestedContainerVTable->pGetElementSize(pIteratorIContainerSrc), pThis->pIAllocator);

				if (status != CSC_STATUS_SUCCESS)
				{
					if (!iterator)
					{
						iterator = CSC_CONTAINER_INVALID_INDEX;
					}
					else
					{
						--iterator;
					}
					break;
				}

				pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * (removeIndex + iterator)), csc_bit_IContainer);
				status = pThis->pNestedContainerVTable->pCopy(pIteratorIContainer, (CONST CSC_IContainer*)pIteratorIContainerSrc);

				if (status != CSC_STATUS_SUCCESS)
				{
					break;
				}
			}

			if (status != CSC_STATUS_SUCCESS)
			{
				if (iterator == CSC_CONTAINER_INVALID_INDEX)
				{
					iterator = removeIndex - (CSC_SIZE_T)1;
				}
				else
				{
					iterator += removeIndex;
				}

				while (iterator >= (CSC_SIZE_T)0 && iterator < removeIndex)
				{
					pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pNewData + pThis->elementSize * iterator), csc_bit_IContainer);
					pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);

					if (!iterator)
					{
						break;
					}

					--iterator;
				}

				CSC_IAllocatorFree(pThis->pIAllocator, pNewData);
				return status;
			}
		}

		for (iterator = (CSC_SIZE_T)0; iterator < pThis->elementCount; ++iterator)
		{
			pIteratorIContainer = (CSC_IContainer*)CSC_IBaseInterfaceGetInterface((CONST CSC_IBaseInterface* CONST)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * iterator), csc_bit_IContainer);
			pThis->pNestedContainerVTable->pDestroy(pIteratorIContainer);
		}
	}
	else
	{
		if (removeIndex)
		{
			status = CSC_MemoryUtilsCopyMemory(pNewData, (CSC_PCVOID)pThis->pData, pThis->elementSize * removeIndex);

			if (status != CSC_STATUS_SUCCESS)
			{
				CSC_IAllocatorFree(pThis->pIAllocator, pNewData);
				return status;
			}
		}

		if (removeIndex + numOfElements < pThis->elementCount)
		{
			status = CSC_MemoryUtilsCopyMemory((CSC_PVOID)((CSC_BYTE* CONST)pNewData + pThis->elementSize * removeIndex), (CSC_PCVOID)((CONST CSC_BYTE* CONST)pThis->pData + pThis->elementSize * (removeIndex + numOfElements)), pThis->elementSize * (pThis->elementCount - (removeIndex + numOfElements)));

			if (status != CSC_STATUS_SUCCESS)
			{
				CSC_IAllocatorFree(pThis->pIAllocator, pNewData);
				return status;
			}
		}
	}

	CSC_IAllocatorFree(pThis->pIAllocator, pThis->pData);

	pThis->pData = pNewData;
	pThis->elementCount -= numOfElements;

	if (pThis->pIIterator)
	{
		CSC_IIteratorOnRemoval(pThis->pIIterator, removeIndex, numOfElements, pThis->elementCount);
	}

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyRemoveRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_DynamicArrayReverse(_Inout_ CSC_DynamicArray* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}


CSC_PVOID CSCMETHOD CSC_DynamicArrayFront(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return NULL;
}

CSC_PVOID CSCMETHOD CSC_DynamicArrayBack(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return NULL;
}

CSC_PVOID CSCMETHOD CSC_DynamicArrayData(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return NULL;
}


CSC_STATUS CSCMETHOD CSC_DynamicArrayIsEmpty(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayIsValid(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return (!pThis || !pThis->elementSize || pThis->baseInterface.pIBaseInterfaceVirtualTable != &g_dynamicArrayVirtualTable.baseInterfaceVTable || pThis->containerInterface.pIContainerVirtualTable != &g_dynamicArrayVirtualTable.containerInterfaceVTable || pThis->iterableInterface.pIIterableVirtualTable != &g_dynamicArrayVirtualTable.iterableInterfaceVTable || !pThis->pIAllocator || pThis->pData && !pThis->reservedSpace || pThis->reservedSpace && !pThis->pData ||(pThis->elementCount * pThis->elementSize) > pThis->reservedSpace || pThis->elementCount > CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE / pThis->elementSize || pThis->elementSize > CSC_DYNAMIC_ARRAY_MAXIMUM_SPACE) ? CSC_STATUS_INVALID_PARAMETER : CSC_STATUS_SUCCESS;
}


CSC_SIZE_T CSCMETHOD CSC_DynamicArrayGetSize(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return CSC_CONTAINER_INVALID_LENGTH;
}

CSC_SIZE_T CSCMETHOD CSC_DynamicArrayGetCapacity(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return CSC_CONTAINER_INVALID_LENGTH;
}

CSC_SIZE_T CSCMETHOD CSC_DynamicArrayGetMaxElements(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return CSC_CONTAINER_INVALID_LENGTH;
}

CSC_SIZE_T CSCMETHOD CSC_DynamicArrayGetElementSize(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return (CSC_SIZE_T)0;
}

CSC_IBaseInterface* CSCMETHOD CSC_DynamicArrayGetIBaseInterface(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return (CSC_IBaseInterface*)NULL;
}

CSC_IContainer* CSCMETHOD CSC_DynamicArrayGetIContainer(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return (CSC_IContainer*)NULL;
}

CSC_IIterable* CSCMETHOD CSC_DynamicArrayGetIIterable(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return (CSC_IIterable*)NULL;
}

CSC_IAllocator* CSCMETHOD CSC_DynamicArrayGetIAllocator(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return (CSC_IAllocator*)NULL;
}

CSC_IContainerVirtualTable* CSCMETHOD CSC_DynamicArrayGetNestedContainerVTable(_In_ CONST CSC_DynamicArray* CONST pThis)
{
	return (CSC_IContainerVirtualTable*)NULL;
}


static CSC_PCVOID CSCMETHOD CSC_DynamicArrayIBaseInterfaceGetInterface(_In_ CONST CSC_IBaseInterface* CONST pThis, _In_ CONST EBaseInterfaceType interfaceType)
{
	CONST CSC_DynamicArray* CONST pDynamicArray = (CONST CSC_DynamicArray* CONST)pThis;

	if (!pDynamicArray || CSC_DynamicArrayIsValid(pDynamicArray) != CSC_STATUS_SUCCESS)
	{
		return (CSC_PCVOID)NULL;
	}

	switch (interfaceType)
	{
	case csc_bit_IContainer:

		return (CSC_PCVOID)&pDynamicArray->containerInterface;

	case csc_bit_IIterable:

		return (CSC_PCVOID)&pDynamicArray->iterableInterface;

	default:

		return (CSC_PCVOID)NULL;
	}

	return (CSC_PCVOID)NULL;
}


static CSC_DynamicArray* CSCMETHOD CSC_DynamicArrayIContainerGetObjectPointer(_In_ CONST CSC_IContainer* CONST pThis)
{
	if (!pThis || pThis->pIContainerVirtualTable != &g_dynamicArrayVirtualTable.containerInterfaceVTable)
	{
		return (CSC_DynamicArray*)NULL;
	}
	else
	{
		return (CSC_DynamicArray*)((CONST CSC_BYTE* CONST)pThis - (CSC_SIZE_T)offsetof(CSC_DynamicArray, containerInterface));
	}
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerInitialize(_Inout_ CONST CSC_PVOID pMemoryBaseAddress, _In_ CONST CSC_SIZE_T elementSize, _In_ CSC_IAllocator* CONST pIAllocator)
{
	if (!pMemoryBaseAddress || !elementSize || !pIAllocator)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_DynamicArrayInitialize(pMemoryBaseAddress, elementSize, pIAllocator, (CONST CSC_IContainerVirtualTable* CONST)NULL);
	}
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerErase(_Inout_ CSC_IContainer* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_DynamicArrayErase(pDynamicArray);
	}
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerDestroy(_Inout_ CSC_IContainer* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_DynamicArrayDestroy(pDynamicArray);
	}
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerCopy(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST struct _CSC_IContainer* CONST pOther)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);
	CSC_DynamicArray* pOtherDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pOther);

	if (!pDynamicArray || !pOtherDynamicArray)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_DynamicArrayCopy(pDynamicArray, pOtherDynamicArray);
	}
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerMove(_Inout_ CSC_IContainer* CONST pThis, _Inout_ struct _CSC_IContainer* CONST pOther)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);
	CSC_DynamicArray* pOtherDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pOther);

	if (!pDynamicArray || !pOtherDynamicArray)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_DynamicArrayMove(pDynamicArray, pOtherDynamicArray);
	}
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerInsertRange(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray || !numOfElements)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_DynamicArrayInsertRange(pDynamicArray, insertIndex, numOfElements, pElements);
	}
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerRemoveRange(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray || !numOfElements)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_DynamicArrayRemoveRange(pDynamicArray, removeIndex, numOfElements);
	}
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerSwapValues(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T secondIndex)
{
	CSC_PVOID pFirstElement, pSecondElement;
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	pFirstElement = CSC_DynamicArrayAccessElement(pDynamicArray, firstIndex);
	pSecondElement = CSC_DynamicArrayAccessElement(pDynamicArray, secondIndex);

	if (!pFirstElement || !pSecondElement)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (firstIndex == secondIndex)
	{
		return CSC_STATUS_SUCCESS;
	}

	return CSC_MemoryUtilsSwapValues(pFirstElement, pSecondElement, pDynamicArray->elementSize, pDynamicArray->pIAllocator);
}

static CSC_PVOID CSCMETHOD CSC_DynamicArrayIContainerAccessElement(_In_ CONST CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T index)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return NULL;
	}
	else
	{
		return CSC_DynamicArrayAccessElement(pDynamicArray, index);
	}
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerIsValid(_In_ CONST CSC_IContainer* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_DynamicArrayIsValid(pDynamicArray);
	}
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerIsEmpty(_In_ CONST CSC_IContainer* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_DynamicArrayIsEmpty(pDynamicArray);
	}
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerIsElementContainer(_In_ CONST CSC_IContainer* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return (CSC_DynamicArrayGetNestedContainerVTable(pDynamicArray) != (CSC_IContainerVirtualTable*)NULL) ? CSC_STATUS_SUCCESS : CSC_STATUS_GENERAL_FAILURE;
	}
}

static CSC_SIZE_T CSCMETHOD CSC_DynamicArrayIContainerGetSize(_In_ CONST CSC_IContainer* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return CSC_CONTAINER_INVALID_LENGTH;
	}
	else
	{
		return CSC_DynamicArrayGetSize(pDynamicArray);
	}
}

static CSC_SIZE_T CSCMETHOD CSC_DynamicArrayIContainerGetElementSize(_In_ CONST CSC_IContainer* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return (CSC_SIZE_T)0;
	}
	else
	{
		return CSC_DynamicArrayGetElementSize(pDynamicArray);
	}
}

static CSC_SIZE_T CSCMETHOD CSC_DynamicArrayIContainerGetMaxElements(_In_ CONST CSC_IContainer* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return CSC_CONTAINER_INVALID_LENGTH;
	}
	else
	{
		return CSC_DynamicArrayGetMaxElements(pDynamicArray);
	}
}

static CSC_IAllocator* CSCMETHOD CSC_DynamicArrayIContainerGetIAllocator(_In_ CONST CSC_IContainer* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return (CSC_IAllocator*)NULL;
	}
	else
	{
		return CSC_DynamicArrayGetIAllocator(pDynamicArray);
	}
}

static CSC_IContainerVirtualTable* CSCMETHOD CSC_DynamicArrayIContainerGetNestedContainerVTable(_In_ CONST struct _CSC_IContainer* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return (CSC_IContainerVirtualTable*)NULL;
	}
	else
	{
		return CSC_DynamicArrayGetNestedContainerVTable(pDynamicArray);
	}
}


static CSC_DynamicArray* CSCMETHOD CSC_DynamicArrayIIterableGetObjectPointer(_In_ CONST  CSC_IIterable* CONST pThis)
{
	if (!pThis || pThis->pIIterableVirtualTable != &g_dynamicArrayVirtualTable.iterableInterfaceVTable)
	{
		return (CSC_DynamicArray*)NULL;
	}
	else
	{
		return (CSC_DynamicArray*)((CONST CSC_BYTE* CONST)pThis - (CSC_SIZE_T)offsetof(CSC_DynamicArray, iterableInterface));
	}
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIIterableRegisterIterator(_Inout_ CSC_IIterable* CONST pThis, _In_ CONST CSC_IIterator* CONST pIIterator)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIIterableGetObjectPointer(pThis);

	if (!pIIterator || !pDynamicArray || CSC_DynamicArrayIsValid(pDynamicArray) != CSC_STATUS_SUCCESS)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pDynamicArray->pIIterator)
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}
	else
	{
		pDynamicArray->pIIterator = pIIterator;
		return CSC_STATUS_SUCCESS;
	}
}

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIIterableUnregisterIterator(_Inout_ CSC_IIterable* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIIterableGetObjectPointer(pThis);

	if (!pDynamicArray || CSC_DynamicArrayIsValid(pDynamicArray) != CSC_STATUS_SUCCESS || !pDynamicArray->pIIterator)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		pDynamicArray->pIIterator = (CSC_IIterator*)NULL;
		return CSC_STATUS_SUCCESS;
	}
}

static CSC_PVOID CSCMETHOD CSC_DynamicArrayIIterableFirstElement(_In_ CONST CSC_IIterable* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIIterableGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return NULL;
	}
	else
	{
		return CSC_DynamicArrayFront(pDynamicArray);
	}
}

static CSC_PVOID CSCMETHOD CSC_DynamicArrayIIterableNextElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIIterableGetObjectPointer(pThis);

	if (!pDynamicArray || currentIndex == CSC_CONTAINER_INVALID_INDEX)
	{
		return NULL;
	}
	else
	{
		return CSC_DynamicArrayAccessElement(pDynamicArray, currentIndex + (CSC_SIZE_T)1);
	}
}

static CSC_PVOID CSCMETHOD CSC_DynamicArrayIIterableLastElement(_In_ CONST CSC_IIterable* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIIterableGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return NULL;
	}
	else
	{
		return CSC_DynamicArrayBack(pDynamicArray);
	}
}

static CSC_PVOID CSCMETHOD CSC_DynamicArrayIIterablePreviousElement(_In_ CONST CSC_IIterable* CONST pThis, _In_ CSC_SIZE_T currentIndex, _In_ CSC_PVOID pCurrentElement)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIIterableGetObjectPointer(pThis);

	if (!pDynamicArray || !currentIndex)
	{
		return NULL;
	}
	else
	{
		return CSC_DynamicArrayAccessElement(pDynamicArray, currentIndex - (CSC_SIZE_T)1);
	}
}

static CSC_PVOID CSCMETHOD CSC_DynamicArrayIIterableGetElementAt(_In_ CONST CSC_IIterable* CONST pThis, _In_ CONST CSC_SIZE_T index, _In_opt_ CONST CSC_SIZE_T currentIndex, _In_opt_ CONST CSC_PVOID pCurrentElement)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIIterableGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return NULL;
	}
	else
	{
		return CSC_DynamicArrayAccessElement(pDynamicArray, index);
	}
}

static CSC_SIZE_T CSCMETHOD CSC_DynamicArrayIIterableGetElementCount(_In_ CONST CSC_IIterable* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIIterableGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return CSC_CONTAINER_INVALID_LENGTH;
	}
	else
	{
		return CSC_DynamicArrayGetSize(pDynamicArray);
	}
}

static CSC_SIZE_T CSCMETHOD CSC_DynamicArrayIIterableGetElementSize(_In_ CONST CSC_IIterable* CONST pThis)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIIterableGetObjectPointer(pThis);

	if (!pDynamicArray)
	{
		return (CSC_SIZE_T)0;
	}
	else
	{
		return CSC_DynamicArrayGetElementSize(pDynamicArray);
	}
}
