#ifndef CSC_WIDE_STRING
#define CSC_WIDE_STRING

#ifdef __cplusplus
extern "C" {
#endif

/*
Description:
    
*/

/* ... */
#include "../DynamicArray/DynamicArray.h"

/* Definition of WString type. */
typedef struct S_CSC_WString
{
    CSC_IBaseInterface baseInterface;
    CSC_IContainer containerInterface;
    CSC_IIterable iterableInterface;
    CSC_IIterator* pIIterator;
    CSC_DynamicArray stringData;
    CSC_DynamicArray indexData;
} CSC_WString;

#ifdef __cplusplus
};
#endif

#endif
