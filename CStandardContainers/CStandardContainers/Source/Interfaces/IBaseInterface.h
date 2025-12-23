#ifndef CSC_BASE_INTERFACE
#define CSC_BASE_INTERFACE

/*
Description:
The IBaseInterface must be implemented by any object in the CSC library that makes use of polymorphism.
Due to the restrictions of C, casting an object to an interface that it implements is not possible.
Hence every such object must implement an instance of IBaseInterface as its first data member, which results in the address of any such object being equal to the address of the underlying IBaseInterface.
*/

// Include the current configuration of the library containing various type definitions and other things.
#include "../Configuration/Configuration.h"

// Definition of all interface types that can be queried through the GetInterface method of IBaseInterface.
// Custom interfaces extending the library can also be added here.
typedef enum _EBaseInterfaceType
{
	csc_bit_IAllocator = 0,
	csc_bit_IContainer,
	csc_bit_IIterator,
	csc_bit_IIterable
} EBaseInterfaceType;

// Predecleration of the CSC_IBaseInterface type to be able to create the function pointer type for the method, that the interface implements.
struct _CSC_IBaseInterface;

// Type definition of the function pointer type of the IBaseInterface's GetInterface method, which needs to be implemented by types that utilize polymorphism.
typedef CSC_PCVOID(CSCMETHOD* CSC_P_I_BASE_INTERFACE_GET_INTERFACE)(_In_ CONST struct _CSC_IBaseInterface* CONST pThis, _In_ CONST EBaseInterfaceType interfaceType);

// Definition of the virtual table layout of the IBaseInterface type.
typedef struct _CSC_IBaseInterfaceVirtualTable
{
	CSC_P_I_BASE_INTERFACE_GET_INTERFACE pGetInterface;
} CSC_IBaseInterfaceVirtualTable;

// Definition of the IBaseInterface type, contains a pointer to the virtual table of the object that implements the interfaces method.
typedef struct _CSC_IBaseInterface
{
	CSC_IBaseInterfaceVirtualTable* pIBaseInterfaceVirtualTable;
} CSC_IBaseInterface;

// Function that can be used to query an interface of given type from an object.
// Returns a nullptr if the object does not contain the interface or if the underlying method couldn't be called due to a defective object being supplied.
// On success the function will return a pointer to an interface in form of a CSC_PCVOID.
CSC_PCVOID CSCMETHOD CSC_IBaseInterfaceGetInterface(_In_ CONST CSC_IBaseInterface* CONST pThis, _In_ CONST EBaseInterfaceType interfaceType);

#endif CSC_BASE_INTERFACE
