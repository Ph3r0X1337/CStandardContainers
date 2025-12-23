#include "IBaseInterface.h"

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