#ifndef CSC_ANSI_STRING
#define CSC_ANSI_STRING

#ifdef __cplusplus
extern "C" {
#endif

/*
Description:
    
*/

/* ... */
#include "../DynamicArray/DynamicArray.h"

/* Definition of AString type. */
typedef struct S_CSC_AString 
{
    CSC_IBaseInterface baseInterface;
    CSC_IContainer containerInterface;
    CSC_IIterable iterableInterface;
    CSC_IIterator *pIIterator;
    CSC_DynamicArray stringData;
    CSC_SIZE_T cachedLength;
} CSC_AString;

#ifdef __cplusplus
};
#endif

#endif
