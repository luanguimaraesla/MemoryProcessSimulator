#ifndef MMS_FIRST_FIT
#define MMS_FIRST_FIT
#include "mms_structures.h"

MemoryCase *divideAndInsert(numberOfSpaces size, priority index, MemoryCase * holeCaseToDivide, Memory *memory);
void removeHoleCase(MemoryCase *toRemove, Memory *memory);
MemoryCase *overwriteHoleCase(priority index, MemoryCase *holeToOverwrite, Memory *memory);
MemoryCase *findNextPriorityListProcessCase(priority index, MemoryCase *firstProcessCase);
MemoryCase *findPrevPriorityListProcessCase(priority index, MemoryCase *firstProcessCase);

#endif
