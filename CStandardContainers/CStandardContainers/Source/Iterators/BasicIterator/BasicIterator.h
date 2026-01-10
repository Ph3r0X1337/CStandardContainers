#ifndef CSC_BASIC_ITERATOR
#define CSC_BASIC_ITERATOR

#ifdef __cplusplus
extern "C" {
#endif

/*
Description:

*/

// Include the current configuration of the library containing various type definitions and other things.
#include "../../Configuration/Configuration.h"
// Include the IBaseInterface as the BasicIterator needs it to implement it's own IIterator interface. 
#include "../../Interfaces/IBaseInterface.h"
// Include the IIterator interface as BasicIterator implements this interface.
#include "../../Interfaces/IIterator.h"
// Include the IIterable interface as the BasicIterator requires working with the IIterable interface.
#include "../../Interfaces/IIterable.h"

typedef struct _CSC_BasicIterator
{
	CSC_IBaseInterface baseInterface;
	CSC_IIterator iteratorInterface;
	CSC_IIterable* pIIterable;
	CSC_SIZE_T elementSize;
	CSC_SIZE_T elementCount;
	CSC_SIZE_T currentIndex;
	CSC_PVOID pCurrentElement;
	CSC_BOOLEAN iterationValid;
} CSC_BasicIterator;



#ifdef __cplusplus
};
#endif

#endif
