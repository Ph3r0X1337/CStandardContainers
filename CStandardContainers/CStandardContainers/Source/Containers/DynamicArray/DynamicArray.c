#include "DynamicArray.h"
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

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerInitialize(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CSC_IAllocator* CONST pIAllocator);
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
		CSC_DynamicArrayIIterableGetElementCount
	}
};

CSC_STATUS CSCMETHOD CSC_DynamicArrayInitialize(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithSize(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithValue(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_PCVOID pValue, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithCopy(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_DynamicArray* CONST pSrc)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithArray(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_PCVOID pFirst, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_DynamicArrayResize(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyResize(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayReserve(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayShrinkToFit(_Inout_ CSC_DynamicArray* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_DynamicArrayDestroy(_Inout_ CSC_DynamicArray* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayErase(_Inout_ CSC_DynamicArray* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayClear(_Inout_ CSC_DynamicArray* CONST pThis)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayZeroMemory(_Out_ CSC_DynamicArray* CONST pThis)
{
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


CSC_STATUS CSCMETHOD CSC_DynamicArrayAssign(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_PCVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayAssignBlock(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_PCVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayAssignRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T lastIndex, _In_ CONST CSC_PCVOID pValue)
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

CSC_STATUS CSCMETHOD CSC_DynamicArrayCopyArray(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_PCVOID pFirst, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T elementSize)
{
	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertElement(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_opt_ CONST CSC_PCVOID pValue)
{
	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pFirst)
{
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
	return CSC_STATUS_SUCCESS;
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

static CSC_STATUS CSCMETHOD CSC_DynamicArrayIContainerInitialize(_Inout_ CSC_IContainer* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CSC_IAllocator* CONST pIAllocator)
{
	CSC_DynamicArray* pDynamicArray = CSC_DynamicArrayIContainerGetObjectPointer(pThis);

	if (!pDynamicArray || !elementSize || !pIAllocator)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_DynamicArrayInitialize(pDynamicArray, elementSize, pIAllocator, (CONST CSC_IContainerVirtualTable* CONST)NULL);
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

