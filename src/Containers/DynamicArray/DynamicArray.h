#ifndef CSC_DYNAMIC_ARRAY
#define CSC_DYNAMIC_ARRAY

#ifdef __cplusplus
extern "C" {
#endif

/*
Description:

The DynamicArray is a type-agnostic container, which stores its elements sequentially in memory.
Functionality-wise, the DynamicArray is similar to the std::vector implementation of the C++ standard library.
This container supports container nesting, hence the elements can also be instances of the DynamicArray type or even other container types.

Through the plethora of methods the user can dynamically modify the contents of the DynamicArray according to his needs:
- Using Push / Pop operations to implement stack-like behaviour (amortized O(1), which means mostly O(1) but O(n) when an operation leads to change in capacity)
- Using Push / PopFront or Insert / Pop operations to implement a queue-like behaviour (slightly slower, O(n))
- Custom logic based on other operations

The container combats the need for reallocations due to growing / shrinking by implementing the following allocation strategy:
- Calculating the correct reserve of memory for the requested size (bigger reserve for allocation on an empty container)
- Doubling the amount of allocated memory if the size grows beyond what memory is reserved (until linear allocation threshold is reached)
- Linear growth by a constant customizable value, which also acts as the threshold where exponential growth turns into linear growth
- Shrinking when the size shrinks below 1/4 of the reserved memory (below the linear allocation threshold) to introduce a hysteresis to exponential growth
- Shrinking when the size shrinks below a value where the difference to the reserved memory is greater than a linear allocation unit to introduce a hysteresis to the linear growth

To instantiate an instance of DynamicArray, you must call one of the DynamicArrayInitialize* methods.
It is important to pass the IContainer virtual table pointer, if the DynamicArray instance is supposed to store elements of the type of that container.
If the elements supposed to be stored are of a POD type (plain old data, no nested resources), you should supply a null pointer for the virtual table pointer parameter.
Keep in mind, that when using container nesting, it is mandatory to provide (a) valid element(s) for any methods that require element insertion.
In contrast, when using the DynamicArray to store POD elements, you can simply provide a null pointer as "default value", 
in which case the memory of the inserted elements will simply be zeroed out.

Once done using an instance of the DynamicArray type, the CSC_DynamicArrayDestroy method should be used to invoke destruction of the object.
Keep in mind that invoking this method will destroy the DynamicArray object, hence you must call one of the DynamicArrayInitialize* methods to use it again.
If the DynamicArray instance should be switched to storing a different type of elements, it is possible to utilize the CSC_DynamicArrayMove,
CSC_DynamicArrayCopy or CSC_DynamicArrayCopyArray method to change the underlying element type and the contents of the container if needed.

The DynamicArray type itself implements the IBaseInterface, the IContainer interface and the IIterable interface.
The IBaseInterface must be implemented by the type as this is the foundation of the runtime-polymorphism built by the CSC library.
Through the IContainer interface DynamicArray exposes all the generic functionality defined by IContainer,
which makes it possible to use DynamicArray instances as generic containers to implement generic logic.
A similar principle applies to the IIterable interface exposed by DynamicArray, it allows a type which properly implements the IIterator interface,
to access any DynamicArray instance as a generic IIterable instance, which allows iteration over the elements without the need to understand the type of the IIterable object.

For utilization of iterators with DynamicArray you have to keep in mind that the container only has a mounting point for a single IIterator instance,
which gets notified by the DynamicArray instance of any container mutation.
However, as the iteration metadata is held by the iterator objects, it is possible to use multiple iterators by decoupling the logic through a composite iterator,
which can act as a proxy for multiple iterators attached to it.

At this point it is important to mention, that the focus of the CSC library are safe, deterministic operations acting upon transactional semantics.
Hence you can rely on this principle: If a method applied to an object fails, the object state either hasn't changed,
or any temporary changes were rolled back to the previous valid state (practically the same behaviour).
Due to this nature of the library, performance wasn't the top priority in some places, as the characteristics above were favoured.
Even though the focus are safe, deterministic operations with transactional semantics, DynamicArray itself is not safe to use in a multithreaded environment.
For such cases it is recommended to either use synchronization primitives to serialize the access to DynamicArray instances,
or create a type which wraps the DynamicArray type and implements a synchronization primitive to serialize access to the container.

Be aware of dangling pointers due to operations on the container that cause reallocations, pointers to prior elements will become dangling pointers.
Hence always make sure to keep track of the operations that you are performing and use the *AccessElement method to update pointers if necessary.
*/

/* Include the current configuration of the library containing various type definitions and other things. */
#include "../../Configuration/Configuration.h"
/* Include the IAllocator interface as the DynamicArray needs access to the interface and its methods. */
#include "../../Interfaces/IAllocator.h"
/* Include the IBaseInterface as the DynamicArray needs it to implement its own interfaces. */
#include "../../Interfaces/IBaseInterface.h"
/* Include the IContainer interface as the DynamicArray implements this interface. */
#include "../../Interfaces/IContainer.h"
/* Include the IIterable interface as the DynamicArray implements this interface. */
#include "../../Interfaces/IIterable.h"
/* Include the IIterator interface as the DynamicArray needs access to the interface and its methods. */
#include "../../Interfaces/IIterator.h"

/* Definition of DynamicArray type. */
typedef struct S_CSC_DynamicArray 
{
    CSC_IBaseInterface baseInterface;
    CSC_IContainer containerInterface;
    CSC_IIterable iterableInterface;
    CONST CSC_IAllocator* pIAllocator;
    CSC_IIterator* pIIterator;
    CONST CSC_IContainerVirtualTable* pNestedContainerVTable;
    CSC_PVOID pData;
    CSC_SIZE_T elementSize;
    CSC_SIZE_T elementCount;
    CSC_SIZE_T reservedSpace;
} CSC_DynamicArray;

/* Basic initialization method, instantiating a DynamicArray instance as an empty container. */
/* Invoking an initialization method on an already initialized instance will fail, just like supplying an element size of zero or an element size bigger than supported. */
/* The method will setup the internal metadata of the DynamicArray instance, it will not allocate any memory as an empty container is requested. */
/* The parameter elementSize expects the size of a single element to store in bytes. */
/* If the element type is not a POD type, the elements must implement the same IContainer interface for resource management. */
/* In such a use case (nested IContainer type) you must pass a pointer of the IContainer virtual table, which the elements will use. */
/* This allows the container to perform runtime type checking for non-POD types. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayInitialize(_When_(return == CSC_STATUS_SUCCESS, _Out_) CSC_DynamicArray *CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable);
/* Initialization method, which instantiates a DynamicArray instance as a container with numOfElements amount of elements. */
/* All the elements are initialized to the default value, which is an optional parameter for POD types. */
/* For POD types, passing a null pointer for the default value will result in the corresponding memory being zeroed. */
/* For nested IContainer elements, passing a valid, initialized default value is mandatory, the same applies to the nested IContainer virtual table pointer parameter. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithSize(_When_(return == CSC_STATUS_SUCCESS, _Out_) CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable);
/* Copy initialization method, which will perform a deep copy on the source object parameter. */
/* This method will make the initialized object use the same IAllocator interface as the source object. */
/* If this behaviour is not desired, you should consider using the basic initialization with the preferred IAllocator, and perform a copy with CSC_DynamicArrayCopy to preserve the IAllocator from initialization. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithCopy(_When_(return == CSC_STATUS_SUCCESS, _Out_) CSC_DynamicArray* CONST pThis, _In_ CONST CSC_DynamicArray* CONST pSrc);
/* Initialization method, which instantiates a DynamicArray instance as a container with numOfElements amount of elements. */
/* This method works similar to the size based initialization, with the difference that instead of passing a default value, you can pass an array of values to initialize the elements. */
/* Again it is possible to pass a null pointer for the pElements parameter for POD types, but for nested IContainer types supplying an array of numOfElements of initialized, valid objects is mandatory. */
/* Just like in the other initialization methods it is mandatory to supply the nested IContainer virtual table pointer for non-POD types. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayInitializeWithArray(_When_(return == CSC_STATUS_SUCCESS, _Out_) CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_PCVOID pElements, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable);

/* Method used to resize the DynamicArray instance, which can either truncate or expand the amount of elements stored by the DynamicArray instance, or change the element capacity. */
/* For POD types, the default element is optional, passing a null pointer will result in the new elements' memory being zeroed out. */
/* For non-POD types is mandatory, if the resize operation will expand the element count of the DynamicArray instance. */
/* However, as a rule of thumb you should always provide a valid default element for non-POD types. */
/* The resize operation will either add new default elements to the "back" of the underlying array, or truncate elements from the back. */
/* Keep in mind that for POD types, shrinking the container will not result in zeroing the memory of truncated elements. */
/* For non-POD types, memory zeroing relies on the IContainer object destructor. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayResize(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue);
/* The same principle applies to the *LazyResize method, however this method will try to prevent unnecessary reallocations. */
/* This means that the DynamicArray's element capacity will not shrink, and it will only be increased if necessary. */
/* That behaviour is optimal for instances, whose element count changes a lot frequently. */
/* This favours performance with the cost of a potentially slightly bigger memory footprint. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyResize(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue);
/* Method used to reserve space for numElements amount of elements, allowing to increase the element capacity of the DynamicArray instance without affecting the element count. */
/* The behaviour is optimal for reserving space for elements when the future element count is either already known or can be roughly estimated. */
/* This allows a big performance optimization by preventing multiple upcoming allocations due to push operations or insertions. */
/* Reserve is only able to expand the capacity, if the calculated capacity is smaller or the same as the current capacity, the method will practically result in a no-op. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayReserve(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements);
/* Method used to shrink the element capacity to the minimal value possible for the current element count of the DynamicArray instance, resulting in a lower memory footprint. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayShrinkToFit(_Inout_ CSC_DynamicArray* CONST pThis);

/* Destructor method for the DynamicArray type. */
/* The method first destroys the elements, if they are of a non-POD type, then it frees the memory and finally destroys the DynamicArray instance itself. */
/* Destruction will fail when used on uninitialized or already destroyed objects. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayDestroy(_Inout_ CSC_DynamicArray* CONST pThis);
/* Method used to destroy all elements and free the allocated memory. */
/* The type metadata is preserved, hence the object stays valid. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayErase(_Inout_ CSC_DynamicArray* CONST pThis);
/* Method used to destroy all elements, in contrast to *Erase it preserves the allocated memory and as such keeps the element capacity. */
/* The type metadata is preserved, hence the object stays valid. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayClear(_Inout_ CSC_DynamicArray* CONST pThis);

/* Method that pushes a single element to the back of the array. */
/* The method allows to implement behaviours such as filling up the array through iteration or implementing a stack- or queue-like behaviour. */
/* Exceeding the current capacity will result in automatic reallocation, just like any method causing element insertion. */
/* For POD types, passing an element is optional, when passing a null pointer the memory of the new element will simply be zeroed out. */
/* For non-POD types using the IContainer interface passing an initialized, valid element is mandatory. */
/* If pushing leads the maximum array size being exceeded, the method will fail. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayPushElement(_Inout_ CSC_DynamicArray* CONST pThis, _In_opt_ CONST CSC_PCVOID pValue);
/* Works similar as *PushElement, with the difference, that the element is pushed to the front of the array. */
/* This difference is coupled to a performance cost, as all elements need to be shifted to the back by one element. */
/* The result is a time complexity of O(n) compared to *PushElement's time complexity of amortized O(1). */
/* Exactly as *PushElement invocation of this method can trigger automatic reallocation to grow the element capacity of the container. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayPushFront(_Inout_ CSC_DynamicArray* CONST pThis, _In_opt_ CONST CSC_PCVOID pValue);
/* Method that pops an element from the back of the array. */
/* The method allows to implement behaviours such as processing an array of work items through iteration or implementing a stack- or queue-like behaviour. */
/* Falling below a threshold of the element count will result in automatic reallocation, to shrink the memory footprint. */
/* The shrinking logic is tied to a built-in hysteresis logic, hence there are no boundaries where frequent push and pop operations will cause reallocations. */
/* The value parameter to receive the element being popped from the back is entirely optional for both POD and non-POD types. */
/* For non-POD types implementing IContainer it is important that you must supply a pointer to an uninitialized object, as the method will initialize the object. */
/* The attempt to pop from an empty or invalid DynamicArray instance will lead to failure. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayPopElement(_Inout_ CSC_DynamicArray* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue);
/* The same behaviour applies to the *LazyPopElement method, with the difference that this operation will not cause any reallocations. */
/* As the pop operation doesn't affect the capacity, this method can be used as a big performance optimization, yet again at the expense of the memory footprint. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyPopElement(_Inout_ CSC_DynamicArray* CONST pThis,_When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue);
/* Works similar as *PopElement, with the difference, that the element is popped from the front of the array. */
/* This difference is coupled to a performance cost, as all elements need to be shifted to the front by one element. */
/* The result is a time complexity of O(n) compared to *PopElement's time complexity of amortized O(1). */
/* Exactly as *PopElement invocation of this method can trigger automatic reallocation to shrink the element capacity of the container. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayPopFront(_Inout_ CSC_DynamicArray* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue);
/* The same behaviour as *PopFront applies here, again with the difference that invocation of this method will not cause any reallocations. */
/* Even though that leads to the capacity not being affected, the time complexity for popping from the front of the array still stays O(n). */
CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyPopFront(_Inout_ CSC_DynamicArray* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue);

/* Method used to access an element of the array at the given index. */
/* If the index is out of bounds, the method returns a null pointer. */
/* Otherwise, the address of the element is returned as type-agnostic pointer of type CSC_PVOID. */
CSC_PVOID CSCMETHOD CSC_DynamicArrayAccessElement(_In_ CONST CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T index);

/* Method used to resize the DynamicArray to numOfElements amount of elements, and set all of the elements to the provided value. */
/* For POD types, the value parameter is optional, passing a null pointer will result in all of the elements' memory being zeroed out. */
/* For non-POD types, it is mandatory to provide a pointer to an initialized, valid object. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayFill(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue);
/* Method used to set a sequential range of elements in the DynamicArray instance to the provided value. */
/* If the specified range goes out of bounds, the method will fail. */
/* For POD types, the provided value parameter is optional, passing a null pointer will result in all of the elements' memory in the range being zeroed out. */
/* For non-POD types, it is mandatory to provide a pointer to an initialized, valid object. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayFillRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue);

/* Method used to create a copy of another DynamicArray instance, keeps the IAllocator but copies everything else from the source object, including the type metadata. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayCopy(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_DynamicArray* CONST pSrc);
/* Method used to move the data from another DynamicArray instance to the current instance, keeps the IAllocator but moves everything else from the source object, including the type metadata. */
/* The allocated data on the other instance gets erased (see CSC_DynamicArrayErase), everything else such as the IAllocator and the type metadata will be preserved. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayMove(_Inout_ CSC_DynamicArray* CONST pThis, _Inout_ CSC_DynamicArray* CONST pSrc);
/* Method used to copy an array of elements in memory into the DynamicArray instance, this method also updates the type metadata. */
/* For POD types, passing the pElements parameter is optional, when passing a null pointer the memory of the new elements will simply be zeroed out. */
/* For non-POD types using the IContainer interface passing an array of initialized, valid elements is mandatory, with a single exception. */
/* Passing a null pointer for the pElements parameter for non-POD types is only supported, when the numOfElements parameter is zero. */
/* In this case the DynamicArray instances elements get destroyed, the allocated memory is freed and the type metadata gets updated. */
/* This essentially transforms the DynamicArray instance into an empty container with the new type metadata. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayCopyArray(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable);

/* Method used to insert a single element into the DynamicArray instance at the given index. */
/* Supplying an index out of bounds results in a failure, however an index equal to the element count is also valid, which leads to insertion at the back of the array. */
/* For POD types, passing an element is optional, when passing a null pointer the memory of the new element will simply be zeroed out. */
/* For non-POD types using the IContainer interface passing an initialized, valid element is mandatory. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertElement(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_opt_ CONST CSC_PCVOID pValue);
/* Method used to insert an array of elements from memory into the DynamicArray instance at the given index. */
/* Supplying an index out of bounds results in a failure, however an index equal to the element count is also valid, which leads to insertion at the back of the array. */
/* For POD types, passing the pElements parameter is optional, when passing a null pointer the memory of the new elements will simply be zeroed out. */
/* For non-POD types using the IContainer interface passing an array of initialized, valid elements is mandatory. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements);
/* Method used to copy and insert the elements of another DynamicArray instance into the current DynamicArray instance at the given index. */
/* Supplying an index out of bounds results in a failure, however an index equal to the element count is also valid, which leads to insertion at the back of the array. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertCopy(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_DynamicArray* CONST pSrc);
/* Method used to move and insert the elements of another DynamicArray instance into the current DynamicArray instance at the given index. */
/* The allocated data on the other instance gets erased (see CSC_DynamicArrayErase), everything else such as the IAllocator and the type metadata will be preserved. */
/* Supplying an index out of bounds results in a failure, however an index equal to the element count is also valid, which leads to insertion at the back of the array. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayInsertMove(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _Inout_ CSC_DynamicArray* CONST pSrc);
/* Method used to insert a copy of another instances elements at the back of the array of the current DynamicArray instance. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayAppendCopy(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_DynamicArray* CONST pSrc);
/* Method used to move the elements of another instance to the current DynamicArray instance, inserting them at the back of the current instances array. */
/* The allocated data on the other instance gets erased (see CSC_DynamicArrayErase), everything else such as the IAllocator and the type metadata will be preserved. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayAppendMove(_Inout_ CSC_DynamicArray* CONST pThis, _Inout_ CSC_DynamicArray* CONST pSrc);

/* Method used to remove a single element from the DynamicArray instance at the given index. */
/* Supplying an index out of bounds results in a failure. */
/* Falling below a threshold of the element count will result in automatic reallocation, to shrink the memory footprint. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayRemoveElement(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex);
/* Method used to remove a single element from the DynamicArray instance at the given index, without affecting the element capacity. */
/* Supplying an index out of bounds results in a failure. */
/* Calling this method will not lead to any reallocations in the DynamicArray instance. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyRemoveElement(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex);
/* Method used to remove a range of elements from the DynamicArray instance at the given index. */
/* Supplying an removeIndex + numOfElements combination that goes out of bounds results in a failure. */
/* Falling below a threshold of the element count will result in automatic reallocation, to shrink the memory footprint. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayRemoveRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements);
/* Method used to remove a range of elements from the DynamicArray instance at the given index, without affecting the element capacity. */
/* Supplying an removeIndex + numOfElements combination that goes out of bounds results in a failure. */
/* Calling this method will not lead to any reallocations in the DynamicArray instance. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayLazyRemoveRange(_Inout_ CSC_DynamicArray* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements);

/* Method used to reverse the order of elements in the given DynamicArray instance. */
/* Calling this method will cause a reallocation to happen, hence the element capacity might be affected, pointers to previous elements will become dangling pointers. */
/* When calling the method on an empty instance or an instance with one element, this will result in a no-op with return value CSC_STATUS_SUCCESS. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayReverse(_Inout_ CSC_DynamicArray* CONST pThis);

/* Method used to retrieve a pointer to the first element of the DynamicArray instance. */
/* The method will return a null pointer, if it fails, which might be caused by an uninitialized or empty DynamicArray instance being passed. */
CSC_PVOID CSCMETHOD CSC_DynamicArrayFront(_In_ CONST CSC_DynamicArray* CONST pThis);
/* Method used to retrieve a pointer to the last element of the DynamicArray instance. */
/* The method will return a null pointer, if it fails, which might be caused by an uninitialized or empty DynamicArray instance being passed. */
CSC_PVOID CSCMETHOD CSC_DynamicArrayBack(_In_ CONST CSC_DynamicArray* CONST pThis);
/* Method used to retrieve the pointer to the underlying allocated memory by the DynamicArray instance. */
/* This method is practically similar to CSC_DynamicArrayFront, but semantically different. */
/* In contrast to CSC_DynamicArrayFront it will also return the pointer to the reserved memory, if the DynamicArray instance is empty but has a non-zero element capacity. */
/* The method will return a null pointer, if it fails, which might be caused by an uninitialized or zero capacity DynamicArray instance being passed. */
CSC_PVOID CSCMETHOD CSC_DynamicArrayData(_In_ CONST CSC_DynamicArray* CONST pThis);

/* Method used to check whether the DynamicArray instance is currently not holding any elements. */
/* Keep in mind that the method returns a value of type CSC_STATUS, which shouldn't be interpreted as a boolean value. */
/* If the instance is empty, the method will return CSC_STATUS_SUCCESS, whereas it will return CSC_STATUS_GENERAL failure, if the instance is holding any elements. */
/* When the supplied instance is invalid, the return value will be CSC_STATUS_INVALID_PARAMETER. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayIsEmpty(_In_ CONST CSC_DynamicArray* CONST pThis);
/* Method used to check whether the DynamicArray instance is a valid, initialized instance. */
/* Keep in mind that the method returns a value of type CSC_STATUS, which shouldn't be interpreted as a boolean value. */
/* If the instance is valid, the method will return CSC_STATUS_SUCCESS, otherwise it will return CSC_STATUS_INVALID_PARAMETER. */
CSC_STATUS CSCMETHOD CSC_DynamicArrayIsValid(_In_ CONST CSC_DynamicArray* CONST pThis);

/* Method used to query the element count of the DynamicArray instance, which represents the number of elements currently stored in the instance. */
/* Supplying an invalid instance will result in the return value CSC_CONTAINER_INVALID_LENGTH. */
CSC_SIZE_T CSCMETHOD CSC_DynamicArrayGetSize(_In_ CONST CSC_DynamicArray* CONST pThis);
/* Method used to query the element capacity of the DynamicArray instance, which represents the number of elements it is able to hold in its allocated reserve. */
/* Supplying an invalid instance will result in the return value CSC_CONTAINER_INVALID_LENGTH. */
CSC_SIZE_T CSCMETHOD CSC_DynamicArrayGetCapacity(_In_ CONST CSC_DynamicArray* CONST pThis);
/* Method used to query the maximum amount of elements that can potentially be stored in the DynamicArray instance, based on the type metadata it holds. */
/* Supplying an invalid instance will result in the return value CSC_CONTAINER_INVALID_LENGTH. */
CSC_SIZE_T CSCMETHOD CSC_DynamicArrayGetMaxElements(_In_ CONST CSC_DynamicArray* CONST pThis);
/* Method used to query the element size in bytes of the DynamicArray instance, which represents the size of a single element based on the type metadata it holds. */
/* Reminder: The type metadata consists of the element size and the pointer to the nested IContainer virtual table, which together build the type information. */
/* Supplying an invalid instance will result in the return value of zero. */
CSC_SIZE_T CSCMETHOD CSC_DynamicArrayGetElementSize(_In_ CONST CSC_DynamicArray* CONST pThis);
/* Method used to obtain a pointer to the IBaseInterface of the DynamicArray instance. */
/* The IBaseInterface pointer can further be used to either pass the instance in a polymorphic way, or query the instance for other interfaces that it implements. */
/* Supplying an invalid instance will result in the return of a null pointer. */
CSC_IBaseInterface* CSCMETHOD CSC_DynamicArrayGetIBaseInterface(_In_ CONST CSC_DynamicArray* CONST pThis);
/* Method used to obtain a pointer to the IContainer interface of the DynamicArray instance. */
/* The IContainer interface pointer can further be used to treat the instance as a generic IContainer instance, such as when implementing type-agnostic sorting algorithms. */
/* Supplying an invalid instance will result in the return of a null pointer. */
CSC_IContainer* CSCMETHOD CSC_DynamicArrayGetIContainer(_In_ CONST CSC_DynamicArray* CONST pThis);
/* Method used to obtain a pointer to the IIterable interface of the DynamicArray instance. */
/* The IIterable interface pointer can further be used to treat the instance as a generic IIterable instance, such as when binding an iterator to the instance. */
/* Supplying an invalid instance will result in the return of a null pointer. */
CSC_IIterable* CSCMETHOD CSC_DynamicArrayGetIIterable(_In_ CONST CSC_DynamicArray* CONST pThis);
/* Method used to obtain the pointer to the IAllocator interface that the instance utilizes. */
/* Supplying an invalid instance will result in the return of a null pointer. */
CSC_IAllocator* CSCMETHOD CSC_DynamicArrayGetIAllocator(_In_ CONST CSC_DynamicArray* CONST pThis);
/* Method used to query the pointer to the nested IContainer virtual table, which is part of the metadata to represent the underlying type. */
/* Reminder: The type metadata consists of the element size and the pointer to the nested IContainer virtual table, which together build the type information. */
/* Supplying an invalid instance will result in the return of a null pointer. */
CONST CSC_IContainerVirtualTable* CSCMETHOD CSC_DynamicArrayGetNestedContainerVTable(_In_ CONST CSC_DynamicArray* CONST pThis);

/* Function used to obtain a pointer to the IContainer virtual table of the DynamicArray type. */
/* As this a function and not a method, a DynamicArray instance is not required to query the pointer to the virtual table. */
/* Querying the pointer is necessary when using the DynamicArray type as a nested container in another container. */
/* Alternatively, a pointer to the IContainer interface of an existing DynamicArray instance can also be used to obtain the pointer, through the IContainer interface. */
CONST CSC_IContainerVirtualTable* CSCAPI CSC_DynamicArrayGetIContainerVTable();

#ifdef __cplusplus
};
#endif

#endif
