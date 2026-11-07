#include "IBaseInterface.h"

CONST CSC_IBaseInterfaceVirtualTable* CSC_IBaseInterfaceGetVTable(_In_ CONST CSC_IBaseInterface* CONST pThis)
{
    return (pThis) ? pThis->pIBaseInterfaceVirtualTable : (CONST CSC_IBaseInterfaceVirtualTable*)NULL;
}

CSC_PCVOID CSCMETHOD CSC_IBaseInterfaceGetInterface(_In_ CONST CSC_IBaseInterface* CONST pThis, _In_ CONST EBaseInterfaceType interfaceType)
{
    if (!pThis || !pThis->pIBaseInterfaceVirtualTable || !pThis->pIBaseInterfaceVirtualTable->pGetInterface)
    {
        return NULL;
    }
    else
    {
        return pThis->pIBaseInterfaceVirtualTable->pGetInterface(pThis, interfaceType);
    }
}