#ifndef CSC_LINKED_LIST
#define CSC_LINKED_LIST

#ifdef __cplusplus
extern "C" {
#endif

/*
Description:

The LinkedList is a type-agnostic container, which stores its elements in separate nodes in memory.
Functionality-wise, the LinkedList is similar to the std::list implementation of the C++ standard library.
However, additionally this doubly linked list also supports a circular mode of operation.
This makes it possible to switch the list from an opened, non-circular list to a closed, circular list at any given point in time and vice-versa.
The LinkedList container supports container nesting, hence the elements can also be instances of the LinkedList type or even other container types.

Through the plethora of methods the user can dynamically modify the contents of the LinkedList according to his needs:
- Using PushFront / PopFront operations to implement stack-like behaviour (O(1) in both closed and opened mode)
- Using Push / PopFront or PushFront / Pop operations to implement a queue-like behaviour (O(1) in closed mode, O(n) in opened mode)
- Custom logic based on other operations

As all elements are stored in separately allocated nodes, there is no strategy being used to combat memory allocations.
This means that essentially every element being inserted corresponds to a memory allocation, just like every element removal corresponds to memory being freed.
It is also worth mentioning that the time complexity of element access is O(n) in the worst case (accessing last element of an opened mode list).
Utilizing a LinkedList in closed mode reduces this complexity on access to O(n/2), other methods will also benefit from the same principle by operating in closed mode.

Another factor that might make this container slow is the higher potential of cache misses due to the bad spatial locality,
caused by the elements potentially being scattered around memory as every element has its own backing memory buffer.
However, this issue can partially be mitigated through the use of a specialized allocator, which allocates the memory for the elements sequentially.

To instantiate an instance of LinkedList, you must call one of the LinkedListInitialize* methods.
It is important to pass the IContainer virtual table pointer, if the LinkedList instance is supposed to store elements of the type of that container.
If the elements supposed to be stored are of a POD type (plain old data, no nested resources), you should supply a null pointer for the virtual table pointer parameter.
Keep in mind, that when using container nesting, it is mandatory to provide (a) valid element(s) for any methods that require element insertion.
In contrast, when using the LinkedList to store POD elements, you can simply provide a null pointer as "default value", 
in which case the memory of the inserted elements will simply be zeroed out.

Once done using an instance of the LinkedList type, the CSC_LinkedListDestroy method should be used to invoke destruction of the object.
Keep in mind that invoking this method will destroy the LinkedList object, hence you must call one of the LinkedListInitialize* methods to use it again.
If the LinkedList instance should be switched to storing a different type of elements, it is possible to utilize the CSC_LinkedListMove,
CSC_LinkedListCopy or CSC_LinkedListCopyArray method to change the underlying element type and the contents of the container if needed.

The LinkedList type itself implements the IBaseInterface, the IContainer interface and the IIterable interface.
The IBaseInterface must be implemented by the type as this is the foundation of the runtime-polymorphism built by the CSC library.
Through the IContainer interface LinkedList exposes all the generic functionality defined by IContainer,
which makes it possible to use LinkedList instances as generic containers to implement generic logic.
A similar principle applies to the IIterable interface exposed by LinkedList, it allows a type which properly implements the IIterator interface,
to access any LinkedList instance as a generic IIterable instance, which allows iteration over the elements without the need to understand the type of the IIterable object.

For utilization of iterators with LinkedList you have to keep in mind that the container only has a mounting point for a single IIterator instance,
which gets notified by the LinkedList instance of any container mutation.
However, as the iteration metadata is held by the iterator objects, it is possible to use multiple iterators by decoupling the logic through a composite iterator,
which can act as a proxy for multiple iterators attached to it.

At this point it is important to mention, that the focus of the CSC library are safe, deterministic operations acting upon transactional semantics.
Hence you can rely on this principle: If a method applied to an object fails, the object state either hasn't changed,
or any temporary changes were rolled back to the previous valid state (practically the same behaviour).
Due to this nature of the library, performance wasn't the top priority in some places, as the characteristics above were favoured.
Even though the focus are safe, deterministic operations with transactional semantics, LinkedList itself is not safe to use in a multithreaded environment.
For such cases it is recommended to either use synchronization primitives to serialize the access to LinkedList instances,
or create a type which wraps the LinkedList type and implements a synchronization primitive to serialize access to the container.

Be aware of dangling pointers due to operations on the container that cause reallocations, pointers to prior elements will become dangling pointers.
Hence always make sure to keep track of the operations that you are performing and use the *AccessElement method to update pointers if necessary.
*/

/* Include the current configuration of the library containing various type definitions and other things. */
#include "../../Configuration/Configuration.h"
/* Include the IAllocator interface as the LinkedList needs access to the interface and its methods. */
#include "../../Interfaces/IAllocator.h"
/* Include the IBaseInterface as the LinkedList needs it to implement its own interfaces. */
#include "../../Interfaces/IBaseInterface.h"
/* Include the IContainer interface as the LinkedList implements this interface. */
#include "../../Interfaces/IContainer.h"
/* Include the IIterable interface as the LinkedList implements this interface. */
#include "../../Interfaces/IIterable.h"
/* Include the IIterator interface as the LinkedList needs access to the interface and its methods. */
#include "../../Interfaces/IIterator.h"

/* Definition of LLNode type. */
typedef struct S_CSC_LLNode
{
    struct S_CSC_LLNode* pNext;
    struct S_CSC_LLNode* pPrevious;
} CSC_LLNode;

/* Definition of LinkedList type. */
typedef struct S_CSC_LinkedList
{
    CSC_IBaseInterface baseInterface;
    CSC_IContainer containerInterface;
    CSC_IIterable iterableInterface;
    CONST CSC_IAllocator* pIAllocator;
    CSC_IIterator* pIIterator;
    CONST CSC_IContainerVirtualTable* pNestedContainerVTable;
    CSC_LLNode* pListHead;
    CSC_SIZE_T elementSize;
    CSC_SIZE_T elementCount;
    CSC_BOOLEAN circular;
} CSC_LinkedList;

/* Basic initialization method, instantiating a LinkedList instance as an empty container. */
/* Invoking an initialization method on an already initialized instance will fail, just like supplying an element size of zero or an element size bigger than supported. */
/* The method will setup the internal metadata of the LinkedList instance, it will not allocate any memory as an empty container is requested. */
/* Through the parameter circular, the LinkedList instance can be initialized in opened mode (FALSE) or closed mode (TRUE). */
/* The parameter elementSize expects the size of a single element to store in bytes. */
/* If the element type is not a POD type, the elements must implement the same IContainer interface for resource management. */
/* In such a use case (nested IContainer type) you must pass a pointer of the IContainer virtual table, which the elements will use. */
/* This allows the container to perform runtime type checking for non-POD types. */
CSC_STATUS CSCMETHOD CSC_LinkedListInitialize(_When_(return == CSC_STATUS_SUCCESS, _Out_) CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_BOOLEAN circular, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable);
/* Initialization method, which instantiates a LinkedList instance as a container with numOfElements amount of elements. */
/* All the elements are initialized to the default value, which is an optional parameter for POD types. */
/* For POD types, passing a null pointer for the default value will result in the corresponding memory being zeroed. */
/* For nested IContainer elements, passing a valid, initialized default value is mandatory, the same applies to the nested IContainer virtual table pointer parameter. */
CSC_STATUS CSCMETHOD CSC_LinkedListInitializeWithSize(_When_(return == CSC_STATUS_SUCCESS, _Out_) CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_BOOLEAN circular, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pDefaultValue, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable);
/* Copy initialization method, which will perform a deep copy on the source object parameter. */
/* This method will make the initialized object use the same IAllocator interface as the source object. */
/* If this behaviour is not desired, you should consider using the basic initialization with the preferred IAllocator, and perform a copy with CSC_LinkedListCopy to preserve the IAllocator from initialization. */
CSC_STATUS CSCMETHOD CSC_LinkedListInitializeWithCopy(_When_(return == CSC_STATUS_SUCCESS, _Out_) CSC_LinkedList* CONST pThis, _In_ CONST CSC_LinkedList* CONST pSrc);
/* Initialization method, which instantiates a LinkedList instance as a container with numOfElements amount of elements. */
/* This method works similar to the size based initialization, with the difference that instead of passing a default value, you can pass an array of values to initialize the elements. */
/* Again it is possible to pass a null pointer for the pElements parameter for POD types, but for nested IContainer types supplying an array of numOfElements of initialized, valid objects is mandatory. */
/* Just like in the other initialization methods it is mandatory to supply the nested IContainer virtual table pointer for non-POD types. */
CSC_STATUS CSCMETHOD CSC_LinkedListInitializeWithArray(_When_(return == CSC_STATUS_SUCCESS, _Out_) CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_BOOLEAN circular, _In_ CONST CSC_SIZE_T numOfElements, _In_ CONST CSC_PCVOID pElements, _In_ CONST CSC_IAllocator* CONST pIAllocator, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable);

/* Destructor method for the LinkedList type. */
/* The method first destroys the elements, if they are of a non-POD type, then it frees the corresponding allocated memory per element and finally destroys the LinkedList instance itself. */
/* Destruction will fail when used on uninitialized or already destroyed objects. */
CSC_STATUS CSCMETHOD CSC_LinkedListDestroy(_Inout_ CSC_LinkedList* CONST pThis);
/* Method used to destroy all elements and free the corresponding allocated memory. */
/* The type metadata is preserved, hence the object stays valid. */
CSC_STATUS CSCMETHOD CSC_LinkedListErase(_Inout_ CSC_LinkedList* CONST pThis);

/* Method used to create a copy of another LinkedList instance, keeps the IAllocator but copies everything else from the source object, including the type metadata. */
CSC_STATUS CSCMETHOD CSC_LinkedListCopy(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_LinkedList* CONST pSrc);
/* Method used to move the data from another LinkedList instance to the current instance, keeps the IAllocator but moves everything else from the source object, including the type metadata. */
/* The allocated data on the other instance gets erased (see CSC_LinkedListErase), everything else such as the IAllocator and the type metadata will be preserved. */
CSC_STATUS CSCMETHOD CSC_LinkedListMove(_Inout_ CSC_LinkedList* CONST pThis, _Inout_ CSC_LinkedList* CONST pSrc);
/* Method used to copy an array of elements in memory into the LinkedList instance, this method also updates the type metadata. */
/* For POD types, passing the pElements parameter is optional, when passing a null pointer the memory of the new elements will simply be zeroed out. */
/* For non-POD types using the IContainer interface passing an array of initialized, valid elements is mandatory, with a single exception. */
/* Passing a null pointer for the pElements parameter for non-POD types is only supported, when the numOfElements parameter is zero. */
/* In this case the LinkedList instances elements get destroyed, the allocated memory for its elements is freed and the type metadata gets updated. */
/* This essentially transforms the LinkedList instance into an empty container with the new type metadata. */
CSC_STATUS CSCMETHOD CSC_LinkedListCopyArray(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_BOOLEAN circular, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements, _In_opt_ CONST CSC_IContainerVirtualTable* CONST pNestedContainerVTable);

/* Method used to resize the LinkedList to numOfElements amount of elements, and set all of the elements to the provided value. */
/* For POD types, the value parameter is optional, passing a null pointer will result in all of the elements' memory being zeroed out. */
/* For non-POD types, it is mandatory to provide a pointer to an initialized, valid object. */
CSC_STATUS CSCMETHOD CSC_LinkedListFill(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue);
/* Method used to set a sequential range of elements in the LinkedList instance to the provided value. */
/* If the specified range goes out of bounds, the method will fail. */
/* For POD types, the provided value parameter is optional, passing a null pointer will result in all of the elements' memory in the range being zeroed out. */
/* For non-POD types, it is mandatory to provide a pointer to an initialized, valid object. */
CSC_STATUS CSCMETHOD CSC_LinkedListFillRange(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T firstIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pValue);

/* Method used to insert a single element into the LinkedList instance at the given index. */
/* Supplying an index out of bounds results in a failure, however an index equal to the element count is also valid, which leads to insertion at the back of the list. */
/* For POD types, passing an element is optional, when passing a null pointer the memory of the new element will simply be zeroed out. */
/* For non-POD types using the IContainer interface passing an initialized, valid element is mandatory. */
CSC_STATUS CSCMETHOD CSC_LinkedListInsertElement(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_opt_ CONST CSC_PCVOID pValue);
/* Method used to insert an array of elements from memory into the LinkedList instance at the given index. */
/* Supplying an index out of bounds results in a failure, however an index equal to the element count is also valid, which leads to insertion at the back of the list. */
/* For POD types, passing the pElements parameter is optional, when passing a null pointer the memory of the new elements will simply be zeroed out. */
/* For non-POD types using the IContainer interface passing an array of initialized, valid elements is mandatory. */
CSC_STATUS CSCMETHOD CSC_LinkedListInsertRange(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_SIZE_T numOfElements, _In_opt_ CONST CSC_PCVOID pElements);
/* Method used to copy and insert the elements of another LinkedList instance into the current LinkedList instance at the given index. */
/* Supplying an index out of bounds results in a failure, however an index equal to the element count is also valid, which leads to insertion at the back of the list. */
CSC_STATUS CSCMETHOD CSC_LinkedListInsertCopy(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _In_ CONST CSC_LinkedList* CONST pOther);
/* Method used to move and insert the elements of another LinkedList instance into the current LinkedList instance at the given index. */
/* The elements on the other instance get destroyed and corresponding memory is freed erased (see CSC_LinkedListErase), everything else such as the IAllocator and the type metadata will be preserved. */
/* Supplying an index out of bounds results in a failure, however an index equal to the element count is also valid, which leads to insertion at the back of the list. */
CSC_STATUS CSCMETHOD CSC_LinkedListInsertMove(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T insertIndex, _Inout_ CSC_LinkedList* CONST pOther);

/* Method used to remove a single element from the LinkedList instance at the given index. */
/* Supplying an index out of bounds results in a failure. */
CSC_STATUS CSCMETHOD CSC_LinkedListRemoveElement(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex);
/* Method used to remove a range of elements from the LinkedList instance at the given index. */
/* Supplying an removeIndex + numOfElements combination that goes out of bounds results in a failure. */
CSC_STATUS CSCMETHOD CSC_LinkedListRemoveRange(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T removeIndex, _In_ CONST CSC_SIZE_T numOfElements);

/* Method that pushes a single element to the back of the list. */
/* The method allows to implement behaviours such as filling up the list through iteration or implementing a stack- or queue-like behaviour. */
/* Utilization of this method results in a time complexity of O(n) in opened mode, as insertion at the back of the list requires walking through all element nodes in this mode. */
/* In closed mode, the time complexity collapses to O(1), as insertion at the back of the list is similar to insertion at the front of the list in this mode. */
/* For POD types, passing an element is optional, when passing a null pointer the memory of the new element will simply be zeroed out. */
/* For non-POD types using the IContainer interface passing an initialized, valid element is mandatory. */
/* If pushing leads the maximum list size being exceeded, the method will fail. */
CSC_STATUS CSCMETHOD CSC_LinkedListPushElement(_Inout_ CSC_LinkedList* CONST pThis, _In_opt_ CONST CSC_PCVOID pValue);
/* Works similar as *PushElement, with the difference, that the element is pushed to the front of the list. */
/* This difference is coupled to an increase in performance when in opened mode, as insertion at the front of the list always operates on a time complexity of O(1). */
CSC_STATUS CSCMETHOD CSC_LinkedListPushFront(_Inout_ CSC_LinkedList* CONST pThis, _In_opt_ CONST CSC_PCVOID pValue);
/* Method that pops an element from the back of the list. */
/* The method allows to implement behaviours such as processing a list of work items through iteration or implementing a stack- or queue-like behaviour. */
/* The value parameter to receive the element being popped from the back is entirely optional for both POD and non-POD types. */
/* For non-POD types implementing IContainer it is important that you must supply a pointer to an uninitialized object, as the method will initialize the object. */
/* The attempt to pop from an empty or invalid LinkedList instance will lead to failure. */
/* Utilization of this method results in a time complexity of O(n) in opened mode, as removing elements from the back of the list requires walking through all element nodes in this mode. */
/* In closed mode, the time complexity collapses to O(1), as removing elements from the back of the list is similar to removing elements from the front of the list in this mode. */
CSC_STATUS CSCMETHOD CSC_LinkedListPopElement(_Inout_ CSC_LinkedList* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue);
/* Works similar as *PopElement, with the difference, that the element is popped from the front of the list. */
/* This difference is coupled to an increase in performance when in opened mode, as removing elements from the front of the list always operates on a time complexity of O(1). */
CSC_STATUS CSCMETHOD CSC_LinkedListPopFront(_Inout_ CSC_LinkedList* CONST pThis, _When_(return == CSC_STATUS_SUCCESS, _Out_opt_) CONST CSC_PVOID pValue);

/* Method used to insert a copy of another instances elements at the back of the list of the current LinkedList instance. */
/* Utilization of this method results in a time complexity of O(n) in opened mode, as insertion at the back of the list requires walking through all element nodes in this mode. */
/* In closed mode, the time complexity collapses to O(1), as insertion at the back of the list is similar to insertion at the front of the list in this mode. */
CSC_STATUS CSCMETHOD CSC_LinkedListAppendCopy(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_LinkedList* CONST pOther);
/* Method used to move the elements of another instance to the current LinkedList instance, inserting them at the back of the current instances list. */
/* The elements on the other instance get destroyed and corresponding memory is freed erased (see CSC_LinkedListErase), everything else such as the IAllocator and the type metadata will be preserved. */
/* Utilization of this method results in a time complexity of O(n) in opened mode, as insertion at the back of the list requires walking through all element nodes in this mode. */
/* In closed mode, the time complexity collapses to O(1), as insertion at the back of the list is similar to insertion at the front of the list in this mode. */
CSC_STATUS CSCMETHOD CSC_LinkedListAppendMove(_Inout_ CSC_LinkedList* CONST pThis, _Inout_ CSC_LinkedList* CONST pOther);

/* Method used to move the head of the list forward by rotationCount amount of elements. */
/* Be aware that moving the head of the list rebases indexing to the new head element (it becomes index 0), which will affect following index-based operations on the LinkedList instance. */
/* The supplied rotationCount parameter doesn't have to be in bounds, the method internally uses modulo of the lists element count to choose the actual rotation count. */
/* Beware: It is only allowed to use this method on a LinkedList instance operating in closed mode (circular), calling this method on an instance operating in opened mode will result in failure. */
/* Due to operating in closed mode, the time complexity of this method is O(n/2) in the worst case scenario. */
/* Providing an empty or invalid LinkedList instance will also result in a failure. */
CSC_STATUS CSCMETHOD CSC_LinkedListRotateForward(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T rotationCount);
/* Method used to move the head of the list forward by a single element. */
/* Be aware that moving the head of the list rebases indexing to the new head element (it becomes index 0), which will affect following index-based operations on the LinkedList instance. */
/* Beware: It is only allowed to use this method on a LinkedList instance operating in closed mode (circular), calling this method on an instance operating in opened mode will result in failure. */
/* Providing an empty or invalid LinkedList instance will also result in a failure. */
CSC_STATUS CSCMETHOD CSC_LinkedListRotateForwardByOne(_Inout_ CSC_LinkedList* CONST pThis);
/* Method used to move the head of the list backward by rotationCount amount of elements. */
/* Be aware that moving the head of the list rebases indexing to the new head element (it becomes index 0), which will affect following index-based operations on the LinkedList instance. */
/* The supplied rotationCount parameter doesn't have to be in bounds, the method internally uses modulo of the lists element count to choose the actual rotation count. */
/* Beware: It is only allowed to use this method on a LinkedList instance operating in closed mode (circular), calling this method on an instance operating in opened mode will result in failure. */
/* Due to operating in closed mode, the time complexity of this method is O(n/2) in the worst case scenario. */
/* Providing an empty or invalid LinkedList instance will also result in a failure. */
CSC_STATUS CSCMETHOD CSC_LinkedListRotateBackward(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T rotationCount);
/* Method used to move the head of the list backward by a single element. */
/* Be aware that moving the head of the list rebases indexing to the new head element (it becomes index 0), which will affect following index-based operations on the LinkedList instance. */
/* Beware: It is only allowed to use this method on a LinkedList instance operating in closed mode (circular), calling this method on an instance operating in opened mode will result in failure. */
/* Providing an empty or invalid LinkedList instance will also result in a failure. */
CSC_STATUS CSCMETHOD CSC_LinkedListRotateBackwardByOne(_Inout_ CSC_LinkedList* CONST pThis);
/* Method used to move the head of the list to the given index. */
/* Be aware that moving the head of the list rebases indexing to the new head element (it becomes index 0), which will affect following index-based operations on the LinkedList instance. */
/* The supplied rotationCount parameter must be in bounds, otherwise calling the method will result in failure. */
/* Beware: It is only allowed to use this method on a LinkedList instance operating in closed mode (circular), calling this method on an instance operating in opened mode will result in failure. */
/* Due to operating in closed mode, the time complexity of this method is O(n/2) in the worst case scenario. */
/* Providing an empty or invalid LinkedList instance will also result in a failure. */
CSC_STATUS CSCMETHOD CSC_LinkedListRotateToIndex(_Inout_ CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T newHeadIndex);

/* Method used to switch the LinkedList instance to operating in opened mode (non-circular). */
/* The method operates on a time complexity of O(1), however keep in mind that this results in a performance cost for other methods due to operating in opened mode. */
/* If the LinkedList instance already operates in opened mode, calling this method results in a no-op with return value CSC_STATUS_SUCCESS. */
CSC_STATUS CSCMETHOD CSC_LinkedListOpen(_Inout_ CSC_LinkedList* CONST pThis);
/* Method used to switch the LinkedList instance to operating in closed mode (circular). */
/* The method operates on a time complexity of O(n), however keep in mind that this results in a performance increase for other methods due to operating in closed mode. */
/* If the LinkedList instance already operates in closed mode, calling this method results in a no-op with return value CSC_STATUS_SUCCESS. */
CSC_STATUS CSCMETHOD CSC_LinkedListClose(_Inout_ CSC_LinkedList* CONST pThis);

/* Method used to access an element of the list at the given index. */
/* If the index is out of bounds, the method returns a null pointer. */
/* Otherwise, the address of the element is returned as type-agnostic pointer of type CSC_PVOID. */
/* It is important to mention that due to the nature of LinkedList, the time complexity on access in opened mode is O(n) worst case. */
/* Operating in closed mode reduces the worst case scenario to a time complexity of O(n/2). */
CSC_PVOID CSCMETHOD CSC_LinkedListAccessElement(_In_ CONST CSC_LinkedList* CONST pThis, _In_ CONST CSC_SIZE_T index);

/* Method used to reverse the order of elements in the given LinkedList instance. */
/* When calling the method on an empty instance or an instance with one element, this will result in a no-op with return value CSC_STATUS_SUCCESS. */
CSC_STATUS CSCMETHOD CSC_LinkedListReverse(_Inout_ CSC_LinkedList* CONST pThis);

/* Method used to retrieve a pointer to the first element of the LinkedList instance. */
/* The method will return a null pointer, if it fails, which might be caused by an uninitialized or empty LinkedList instance being passed. */
/* This method operates on a time complexity of O(1) in both opened and closed mode. */
CSC_PVOID CSCMETHOD CSC_LinkedListFront(_In_ CONST CSC_LinkedList* CONST pThis);
/* Method used to retrieve a pointer to the last element of the LinkedList instance. */
/* The method will return a null pointer, if it fails, which might be caused by an uninitialized or empty LinkedList instance being passed. */
/* This method operates on a time complexity of O(n) in opened mode, while in closed mode the complexity is O(1) just like accessing the front element. */
CSC_PVOID CSCMETHOD CSC_LinkedListBack(_In_ CONST CSC_LinkedList* CONST pThis);

/* Method used to check whether the LinkedList instance is currently not holding any elements. */
/* Keep in mind that the method returns a value of type CSC_STATUS, which shouldn't be interpreted as a boolean value. */
/* If the instance is empty, the method will return CSC_STATUS_SUCCESS, whereas it will return CSC_STATUS_GENERAL failure, if the instance is holding any elements. */
/* When the supplied instance is invalid, the return value will be CSC_STATUS_INVALID_PARAMETER. */
CSC_STATUS CSCMETHOD CSC_LinkedListIsEmpty(_In_ CONST CSC_LinkedList* CONST pThis);
/* Method used to check whether the LinkedList instance is a valid, initialized instance. */
/* Keep in mind that the method returns a value of type CSC_STATUS, which shouldn't be interpreted as a boolean value. */
/* If the instance is valid, the method will return CSC_STATUS_SUCCESS, otherwise it will return CSC_STATUS_INVALID_PARAMETER. */
CSC_STATUS CSCMETHOD CSC_LinkedListIsValid(_In_ CONST CSC_LinkedList* CONST pThis);
/* Method used to check whether the LinkedList instance is currently operating in closed mode (circular) or not (opened mode, non-circular). */
/* Keep in mind that the method returns a value of type CSC_STATUS, which shouldn't be interpreted as a boolean value. */
/* If the instance is circular (closed mode), the method will return CSC_STATUS_SUCCESS, whereas it will return CSC_STATUS_GENERAL failure, if the instance is a non-circular LinkedList (opened mode). */
/* When the supplied instance is invalid, the return value will be CSC_STATUS_INVALID_PARAMETER. */
CSC_STATUS CSCMETHOD CSC_LinkedListIsCircular(_In_ CONST CSC_LinkedList* CONST pThis);

/* Method used to query the element count of the LinkedList instance, which represents the number of elements currently stored in the instance. */
/* Supplying an invalid instance will result in the return value CSC_CONTAINER_INVALID_LENGTH. */
CSC_SIZE_T CSCMETHOD CSC_LinkedListGetSize(_In_ CONST CSC_LinkedList* CONST pThis);
/* Method used to query the maximum amount of elements that can potentially be stored in the LinkedList instance, based on the type metadata it holds. */
/* Supplying an invalid instance will result in the return value CSC_CONTAINER_INVALID_LENGTH. */
CSC_SIZE_T CSCMETHOD CSC_LinkedListGetMaxElements(_In_ CONST CSC_LinkedList* CONST pThis);
/* Method used to query the element size in bytes of the LinkedList instance, which represents the size of a single element based on the type metadata it holds. */
/* Keep in mind that the element size does not include the overhead of the LinkedList node, which is the actual size of bytes in memory a single element consumes. */
/* Reminder: The type metadata consists of the element size and the pointer to the nested IContainer virtual table, which together build the type information. */
/* Supplying an invalid instance will result in the return value of zero. */
CSC_SIZE_T CSCMETHOD CSC_LinkedListGetElementSize(_In_ CONST CSC_LinkedList* CONST pThis);
/* Method used to obtain a pointer to the IBaseInterface of the LinkedList instance. */
/* The IBaseInterface pointer can further be used to either pass the instance in a polymorphic way, or query the instance for other interfaces that it implements. */
/* Supplying an invalid instance will result in the return of a null pointer. */
CSC_IBaseInterface* CSCMETHOD CSC_LinkedListGetIBaseInterface(_In_ CONST CSC_LinkedList* CONST pThis);
/* Method used to obtain a pointer to the IContainer interface of the LinkedList instance. */
/* The IContainer interface pointer can further be used to treat the instance as a generic IContainer instance, such as when implementing type-agnostic sorting algorithms. */
/* Supplying an invalid instance will result in the return of a null pointer. */
CSC_IContainer* CSCMETHOD CSC_LinkedListGetIContainer(_In_ CONST CSC_LinkedList* CONST pThis);
/* Method used to obtain a pointer to the IIterable interface of the LinkedList instance. */
/* The IIterable interface pointer can further be used to treat the instance as a generic IIterable instance, such as when binding an iterator to the instance. */
/* Supplying an invalid instance will result in the return of a null pointer. */
CSC_IIterable* CSCMETHOD CSC_LinkedListGetIIterable(_In_ CONST CSC_LinkedList* CONST pThis);
/* Method used to obtain the pointer to the IAllocator interface that the instance utilizes. */
/* Supplying an invalid instance will result in the return of a null pointer. */
CSC_IAllocator* CSCMETHOD CSC_LinkedListGetIAllocator(_In_ CONST CSC_LinkedList* CONST pThis);
/* Method used to query the pointer to the nested IContainer virtual table, which is part of the metadata to represent the underlying type. */
/* Reminder: The type metadata consists of the element size and the pointer to the nested IContainer virtual table, which together build the type information. */
/* Supplying an invalid instance will result in the return of a null pointer. */
CONST CSC_IContainerVirtualTable* CSCMETHOD CSC_LinkedListGetNestedContainerVTable(_In_ CONST CSC_LinkedList* CONST pThis);

/* Function used to obtain a pointer to the IContainer virtual table of the LinkedList type. */
/* As this a function and not a method, a LinkedList instance is not required to query the pointer to the virtual table. */
/* Querying the pointer is necessary when using the LinkedList type as a nested container in another container. */
/* Alternatively, a pointer to the IContainer interface of an existing LinkedList instance can also be used to obtain the pointer, through the IContainer interface. */
CONST CSC_IContainerVirtualTable* CSCAPI CSC_LinkedListGetIContainerVTable();

#ifdef __cplusplus
};
#endif

#endif
