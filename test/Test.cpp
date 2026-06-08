#include <CStandardContainers.h>

#include "Allocator/StandardAllocator.h"

#include <print>

struct IVec3 {
  int x;
  int y;
  int z;
};

#define SIZE_T size_t

void printVec(const CSC_DynamicArray *const pVec) {
  SIZE_T iterator;
  IVec3 *pElement;

  std::println("Printing vector contents:");

  for (iterator = (SIZE_T)0; iterator < CSC_DynamicArrayGetSize(pVec);
       ++iterator) {
    pElement = (IVec3 *)CSC_DynamicArrayAccessElement(pVec, iterator);

    if (!pElement) {
      std::println("Accessing element number {} failed!\n", iterator);
      return;
    } else {
      std::println("Element Nr.{}: X: {}, Y: {}, Z: {}\n", iterator,
                   pElement->x, pElement->y, pElement->z);
    }
  }

  std::println("\n");
}

int main() {
  const CSC_IAllocator *pAllocator = CSC_StandardAllocatorGetAllocator();
  CSC_DynamicArray da;
  IVec3 vecBuffer;
  IVec3 *pVecInArr;
  SIZE_T iterator;
  const SIZE_T vecSize = (SIZE_T)15;

  if (!pAllocator) {
    std::println("Couldnt get allocator");
    return 1;
  }

  if (CSC_DynamicArrayInitialize(&da, sizeof(IVec3), pAllocator, nullptr) !=
      CSC_STATUS_SUCCESS) {
    std::println("Failed to initialize vector!");
    return 1;
  }

  std::println("Vector Size: {}   Vector Reserve: {}   Vector Bytesize: {}\n",
               (int)CSC_DynamicArrayGetSize(&da),
               (int)CSC_DynamicArrayGetCapacity(&da),
               (int)(CSC_DynamicArrayGetSize(&da) *
                     CSC_DynamicArrayGetElementSize(&da)));

  std::println("Creation of vector elements:\n");

  for (iterator = (size_t)0; iterator < vecSize; ++iterator) {
    vecBuffer.x = (int)iterator + 1;
    vecBuffer.y = 0;
    vecBuffer.z = (int)(vecSize - iterator);

    if (CSC_DynamicArrayPushValue(&da, &vecBuffer) != CSC_STATUS_SUCCESS)
      std::println("Failed to push value at {}", iterator);
  }
  std::println("Vector Size: {}   Vector Reserve: {}   Vector Bytesize: {}",
               (int)CSC_DynamicArrayGetSize(&da),
               (int)/*CSC_DynamicArrayGetCapacity(&da)*/ da.reservedSpace,
               (int)(CSC_DynamicArrayGetSize(&da) *
                     CSC_DynamicArrayGetElementSize(&da)));
  printVec(&da);

  std::println("Insertion of vector elements:\n");

  CSC_DynamicArrayInsertRange(&da, 5, 10, &vecBuffer);

  std::println("Vector Size: {}   Vector Reserve: {}   Vector Bytesize: {}\n",
               (int)CSC_DynamicArrayGetSize(&da),
               (int)/*CSC_DynamicArrayGetCapacity(&da)*/ da.reservedSpace,
               (int)(CSC_DynamicArrayGetSize(&da) *
                     CSC_DynamicArrayGetElementSize(&da)));
  printVec(&da);

  std::println("Removal of vector elements:\n");

  CSC_DynamicArrayRemoveRange(&da, 5, 10);

  std::println("Vector Size: {}   Vector Reserve: {}   Vector Bytesize: {}",
               (int)CSC_DynamicArrayGetSize(&da),
               (int)/*CSC_DynamicArrayGetCapacity(&da)*/ da.reservedSpace,
               (int)(CSC_DynamicArrayGetSize(&da) *
                     CSC_DynamicArrayGetElementSize(&da)));
  printVec(&da);

  std::println("Reversing the order of elements:\n");

  CSC_DynamicArrayReverse(&da);

  std::println("Vector Size: {}   Vector Reserve: {}   Vector Bytesize: {}",
               (int)CSC_DynamicArrayGetSize(&da),
               (int)/*CSC_DynamicArrayGetCapacity(&da)*/ da.reservedSpace,
               (int)(CSC_DynamicArrayGetSize(&da) *
                     CSC_DynamicArrayGetElementSize(&da)));
  printVec(&da);

  std::println("Deletion of vector elements:\n");

  for (iterator = (SIZE_T)0; iterator < vecSize; ++iterator) {
    if (CSC_DynamicArrayPopValue(&da, &vecBuffer) != CSC_STATUS_SUCCESS)
      std::println("Failed to pop a value at {}", iterator);
  }
  std::println("Vector Size: {}   Vector Reserve: {}   Vector Bytesize: {}",
               (int)CSC_DynamicArrayGetSize(&da),
               (int)/*CSC_DynamicArrayGetCapacity(&da)*/ da.reservedSpace,
               (int)(CSC_DynamicArrayGetSize(&da) *
                     CSC_DynamicArrayGetElementSize(&da)));
  printVec(&da);

  IVec3 def{0, 0, 0};
  CSC_DynamicArrayResize(&da, (SIZE_T)20, &def);

  std::println("Vector Size: {}   Vector Reserve: {}   Vector Bytesize: {}",
               (int)CSC_DynamicArrayGetSize(&da),
               (int)/*CSC_DynamicArrayGetCapacity(&da)*/ da.reservedSpace,
               (int)(CSC_DynamicArrayGetSize(&da) *
                     CSC_DynamicArrayGetElementSize(&da)));
  printVec(&da);

  vecBuffer = (IVec3){1337, 1337, 1337};

  pVecInArr = (IVec3 *)CSC_DynamicArrayAccessElement(&da, 19);

  if (!pVecInArr) {
    std::println("Failed to obtain pointer to an object in the vector!");
  } else {
    pVecInArr->x = vecBuffer.x;
    pVecInArr->y = vecBuffer.y;
    pVecInArr->z = vecBuffer.z;

    printVec(&da);
  }

  pVecInArr = (IVec3 *)CSC_DynamicArrayAccessElement(&da, 11);

  if (!pVecInArr) {
    std::println("Failed to obtain pointer to an object in the vector!");
  } else {
    pVecInArr->x = vecBuffer.x;
    pVecInArr->y = vecBuffer.y;
    pVecInArr->z = vecBuffer.z;

    printVec(&da);
  }

  std::println("Inserting element at the front:\n");

  vecBuffer = (IVec3){6502, 6502, 6502};

  if (CSC_DynamicArrayInsertElement(&da, 0, &vecBuffer) != CSC_STATUS_SUCCESS) {
    std::println("Failed to insert an element at the front of the vector!");
  } else {
    std::println("Vector Size: {}   Vector Reserve: {}   Vector Bytesize: {}",
                 (int)CSC_DynamicArrayGetSize(&da),
                 (int)/*CSC_DynamicArrayGetCapacity(&da)*/ da.reservedSpace,
                 (int)(CSC_DynamicArrayGetSize(&da) *
                       CSC_DynamicArrayGetElementSize(&da)));
    printVec(&da);
  }

  pVecInArr = (IVec3 *)CSC_DynamicArrayFront(&da);

  if (!pVecInArr) {
    std::println("Failed to obtain pointer to the first object in the vector!");
  } else {
    pVecInArr->x = 1337;
    pVecInArr->y = 1337;
    pVecInArr->z = 1337;

    printVec(&da);
  }

  std::println("Popping element from the front:\n");
  std::println("Value before popping: X: {}, Y: {}, Z: {}\n", vecBuffer.x,
               vecBuffer.y, vecBuffer.z);

  vecBuffer = (IVec3){6502, 6502, 6502};

  if (CSC_DynamicArrayPopFront(&da, &vecBuffer) != CSC_STATUS_SUCCESS) {
    std::println("Failed to pop an element at the front of the vector!");
  } else {
    std::println("Popped value: X: {}, Y: {}, Z: {}", vecBuffer.x, vecBuffer.y,
                 vecBuffer.z);

    std::println("Vector Size: {}   Vector Reserve: {}   Vector Bytesize: {}",
                 (int)CSC_DynamicArrayGetSize(&da),
                 (int)/*CSC_DynamicArrayGetCapacity(&da)*/ da.reservedSpace,
                 (int)(CSC_DynamicArrayGetSize(&da) *
                       CSC_DynamicArrayGetElementSize(&da)));
    printVec(&da);
  }

  CSC_DynamicArrayErase(&da);

  return 0;
}
