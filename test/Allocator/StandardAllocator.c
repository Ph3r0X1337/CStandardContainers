#include "StandardAllocator.h"
#include <memory.h>
#include <stdlib.h>

typedef struct _CSC_StandardAllocatorVirtualTable {
  CSC_IBaseInterfaceVirtualTable baseInterfaceVTable;
  CSC_IAllocatorVirtualTable allocatorInterfaceVTable;
} CSC_StandardAllocatorVirtualTable;

typedef struct _CSC_StandardAllocator {
  CSC_IBaseInterface baseInterface;
  CSC_IAllocator allocatorInterface;
} CSC_StandardAllocator;

static CSC_PCVOID CSCMETHOD CSC_StandardAllocatorIBaseInterfaceGetInterface(
    _In_ CONST CSC_IBaseInterface *CONST pThis,
    _In_ CONST EBaseInterfaceType interfaceType);

static CSC_STATUS CSCMETHOD
CSC_StandardAllocatorIAllocatorInit(_Inout_ CSC_IAllocator *CONST pThis);
static CSC_STATUS CSCMETHOD
CSC_StandardAllocatorIAllocatorCleanup(_Inout_ CSC_IAllocator *CONST pThis);
static CSC_PVOID CSCMETHOD CSC_StandardAllocatorIAllocatorAlloc(
    _In_ CONST CSC_IAllocator *CONST pThis, _In_ CONST CSC_SIZE_T size);
static CSC_PVOID CSCMETHOD CSC_StandardAllocatorIAllocatorAllocZero(
    _In_ CONST CSC_IAllocator *CONST pThis, _In_ CONST CSC_SIZE_T size);
static CSC_STATUS CSCMETHOD CSC_StandardAllocatorIAllocatorFree(
    _In_ CONST CSC_IAllocator *CONST pThis, _In_ CONST CSC_PVOID pMemoryBlock);
static CSC_BOOLEAN CSCMETHOD
CSC_StandardAllocatorIAllocatorIsUsable(_In_ CONST CSC_IAllocator *CONST pThis);

static CONST CSC_StandardAllocatorVirtualTable g_standardAllocatorVirtualTable =
    {{CSC_StandardAllocatorIBaseInterfaceGetInterface},
     {CSC_StandardAllocatorIAllocatorInit,
      CSC_StandardAllocatorIAllocatorCleanup,
      CSC_StandardAllocatorIAllocatorAlloc,
      CSC_StandardAllocatorIAllocatorAllocZero,
      CSC_StandardAllocatorIAllocatorFree,
      CSC_StandardAllocatorIAllocatorIsUsable}};

static CSC_StandardAllocator g_standardAllocator = {
    {(CSC_IBaseInterfaceVirtualTable *)&g_standardAllocatorVirtualTable
         .baseInterfaceVTable},
    {(CSC_IAllocatorVirtualTable *)&g_standardAllocatorVirtualTable
         .allocatorInterfaceVTable}};

CSC_IAllocator *CSCAPI CSC_StandardAllocatorGetAllocator() {
  return &g_standardAllocator.allocatorInterface;
}

static CSC_PCVOID CSCMETHOD CSC_StandardAllocatorIBaseInterfaceGetInterface(
    _In_ CONST CSC_IBaseInterface *CONST pThis,
    _In_ CONST EBaseInterfaceType interfaceType) {
  CONST CSC_StandardAllocator *CONST pStandardAllocator =
      (CONST CSC_StandardAllocator * CONST) pThis;

  if (pStandardAllocator != &g_standardAllocator) {
    return (CSC_PCVOID)NULL;
  }

  switch (interfaceType) {
  case csc_bit_IAllocator:

    return (CSC_PCVOID)&pStandardAllocator->allocatorInterface;

  default:

    return (CSC_PCVOID)NULL;
  }

  return (CSC_PCVOID)NULL;
}

static CSC_STATUS CSCMETHOD
CSC_StandardAllocatorIAllocatorInit(_Inout_ CSC_IAllocator *CONST pThis) {
  return (pThis != &g_standardAllocator.allocatorInterface)
             ? CSC_STATUS_INVALID_PARAMETER
             : CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCMETHOD
CSC_StandardAllocatorIAllocatorCleanup(_Inout_ CSC_IAllocator *CONST pThis) {
  return (pThis != &g_standardAllocator.allocatorInterface)
             ? CSC_STATUS_INVALID_PARAMETER
             : CSC_STATUS_SUCCESS;
}

static CSC_PVOID CSCMETHOD CSC_StandardAllocatorIAllocatorAlloc(
    _In_ CONST CSC_IAllocator *CONST pThis, _In_ CONST CSC_SIZE_T size) {
  if (pThis != &g_standardAllocator.allocatorInterface) {
    return NULL;
  } else {
    return (CSC_PVOID)malloc((size_t)size);
  }
}

static CSC_PVOID CSCMETHOD CSC_StandardAllocatorIAllocatorAllocZero(
    _In_ CONST CSC_IAllocator *CONST pThis, _In_ CONST CSC_SIZE_T size) {
  if (pThis != &g_standardAllocator.allocatorInterface) {
    return NULL;
  } else {
    return (CSC_PVOID)calloc((size_t)size, (size_t)1);
  }
}

static CSC_STATUS CSCMETHOD CSC_StandardAllocatorIAllocatorFree(
    _In_ CONST CSC_IAllocator *CONST pThis, _In_ CONST CSC_PVOID pMemoryBlock) {
  if (pThis != &g_standardAllocator.allocatorInterface) {
    return CSC_STATUS_INVALID_PARAMETER;
  } else {
    free((void *)pMemoryBlock);
    return CSC_STATUS_SUCCESS;
  }
}

static CSC_BOOLEAN CSCMETHOD CSC_StandardAllocatorIAllocatorIsUsable(
    _In_ CONST CSC_IAllocator *CONST pThis) {
  return (CSC_BOOLEAN)((pThis != &g_standardAllocator.allocatorInterface)
                           ? FALSE
                           : TRUE);
}
