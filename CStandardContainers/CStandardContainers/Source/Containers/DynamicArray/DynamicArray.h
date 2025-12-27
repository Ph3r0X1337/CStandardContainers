#ifndef CSC_DYNAMIC_ARRAY
#define CSC_DYNAMIC_ARRAY

#include "../../Configuration/Configuration.h"
#include "../../Interfaces/IBaseInterface.h"
#include "../../Interfaces/IAllocator.h"
#include "../../Interfaces/IContainer.h"
#include "../../Interfaces/IIterator.h"
#include "../../Interfaces/IIterable.h"

typedef struct _CSC_DynamicArray
{
	CSC_IBaseInterface baseInterface;
	CSC_IContainer containerInterface;
	CSC_IIterable iterableInterface;
	CSC_IAllocator* pIAllocator;
	CSC_IIterator* pIIterator;
	CSC_IContainerVirtualTable* pNestedContainerVTable;
	CSC_PVOID pData;
	CSC_SIZE_T elementSize;
	CSC_SIZE_T elementCount;
	CSC_SIZE_T reservedSpace;
} CSC_DynamicArray;

CSC_STATUS CSCMETHOD CSC_DynamicArrayInitialize(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable);
CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithSize(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable);
CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithValue(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_PCVOID pValue, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable);
CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithCopy(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_DynamicArray* CONST pSrc);
CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithArray(_Out_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_PCVOID pFirst, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable);

CSC_STATUS CSCMETHOD CSC_DynamicArrayResize(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue);
CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyResize(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue);
CSC_STATUS CSCMETHOD CSC_DynamicArrayReserve(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements);
CSC_STATUS CSCMETHOD CSC_DynamicArrayShrinkToFit(_Inout_ CSC_DynamicArray* CONST pThis);

CSC_STATUS CSCMETHOD CSC_DynamicArrayDestroy(_Inout_ CSC_DynamicArray* CONST pThis);
CSC_STATUS CSCMETHOD CSC_DynamicArrayErase(_Inout_ CSC_DynamicArray* CONST pThis);
CSC_STATUS CSCMETHOD CSC_DynamicArrayClear(_Inout_ CSC_DynamicArray* CONST pThis);
CSC_STATUS CSCMETHOD CSC_DynamicArrayZeroMemory(_Out_ CSC_DynamicArray* CONST pThis);

CSC_STATUS CSCMETHOD CSC_DynamicArrayPushValue(_Inout_ CSC_DynamicArray* CONST pThis, _In_opt_ CONST CSC_PCVOID pValue);
CSC_STATUS CSCMETHOD CSC_DynamicArrayPopValue(_Inout_ CSC_DynamicArray* CONST pThis, _When_(return == STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue);
CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyPopValue(_Inout_ CSC_DynamicArray* CONST pThis, _When_(return == STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue);
CSC_STATUS CSCMETHOD CSC_DynamicArrayPopFront(_Inout_ CSC_DynamicArray* CONST pThis, _When_(return == STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue);
CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyPopFront(_Inout_ CSC_DynamicArray* CONST pThis, _When_(return == STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue);

CSC_PVOID CSCMETHOD CSC_DynamicArrayAccessElement(_In_ CONST CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T index);

CSC_STATUS CSCMETHOD CSC_DynamicArrayAssign(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_PCVOID pValue);
CSC_STATUS CSCMETHOD CSC_DynamicArrayAssignBlock(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_PCVOID pValue);
CSC_STATUS CSCMETHOD CSC_DynamicArrayAssignRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T lastIndex, _In_ CONST CSC_PCVOID pValue);

CSC_STATUS CSCMETHOD CSC_DynamicArrayCopy(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_DynamicArray* CONST pSrc);
CSC_STATUS CSCMETHOD CSC_DynamicArrayMove(_Inout_ CSC_DynamicArray* CONST pThis, _Inout_ CSC_DynamicArray* CONST pSrc);
CSC_STATUS CSCMETHOD CSC_DynamicArrayCopyArray(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_PCVOID pFirst, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_SIZE_T elementSize);

CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertElement(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_opt_ CONST CSC_PCVOID pValue);
CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_opt_ CONST CSC_PCVOID pFirst, _In_ CONST CSC_SIZE_T numOfElements);
CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertArray(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_DynamicArray* CONST pSrc);
CSC_STATUS CSCMETHOD CSC_DynamicArrayAppendCopy(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_DynamicArray* CONST pSrc);
CSC_STATUS CSCMETHOD CSC_DynamicArrayAppendMove(_Inout_ CSC_DynamicArray* CONST pThis, _Inout_ CSC_DynamicArray* CONST pSrc);

CSC_STATUS CSCMETHOD CSC_DynamicArrayRemoveElement(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex);
CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyRemoveElement(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex);
CSC_STATUS CSCMETHOD CSC_DynamicArrayRemoveRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements);
CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyRemoveRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements);

CSC_STATUS CSCMETHOD CSC_DynamicArrayReverse(_Inout_ CSC_DynamicArray* CONST pThis);

CSC_PVOID CSCMETHOD CSC_DynamicArrayFront(_In_ CONST CSC_DynamicArray* CONST pThis);
CSC_PVOID CSCMETHOD CSC_DynamicArrayBack(_In_ CONST CSC_DynamicArray* CONST pThis);
CSC_PVOID CSCMETHOD CSC_DynamicArrayData(_In_ CONST CSC_DynamicArray* CONST pThis);

CSC_STATUS CSCMETHOD CSC_DynamicArrayIsEmpty(_In_ CONST CSC_DynamicArray* CONST pThis);
CSC_STATUS CSCMETHOD CSC_DynamicArrayIsValid(_In_ CONST CSC_DynamicArray* CONST pThis);

CSC_SIZE_T CSCMETHOD CSC_DynamicArrayGetSize(_In_ CONST CSC_DynamicArray* CONST pThis);
CSC_SIZE_T CSCMETHOD CSC_DynamicArrayGetCapacity(_In_ CONST CSC_DynamicArray* CONST pThis);
CSC_SIZE_T CSCMETHOD CSC_DynamicArrayGetMaxElements(_In_ CONST CSC_DynamicArray* CONST pThis);
CSC_SIZE_T CSCMETHOD CSC_DynamicArrayGetElementSize(_In_ CONST CSC_DynamicArray* CONST pThis);
CSC_IBaseInterface* CSCMETHOD CSC_DynamicArrayGetIBaseInterface(_In_ CONST CSC_DynamicArray* CONST pThis);
CSC_IContainer* CSCMETHOD CSC_DynamicArrayGetIContainer(_In_ CONST CSC_DynamicArray* CONST pThis);
CSC_IIterable* CSCMETHOD CSC_DynamicArrayGetIIterable(_In_ CONST CSC_DynamicArray* CONST pThis);
CSC_IAllocator* CSCMETHOD CSC_DynamicArrayGetIAllocator(_In_ CONST CSC_DynamicArray* CONST pThis);
CSC_IContainerVirtualTable* CSCMETHOD CSC_DynamicArrayGetNestedContainerVTable(_In_ CONST CSC_DynamicArray* CONST pThis);

#endif