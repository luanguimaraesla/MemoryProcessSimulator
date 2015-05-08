#ifndef MMS_WORST_FIT
#define MMS_WORST_FIT
#include "mms_structures.h"

MemoryCase *findTheWorstFitHoleCase(numberOfSpaces size, Memory *memory);
MemoryCase *reallocAndInsert_worst(numberOfSpaces size, priority index, MemoryCase *insertBegin, Memory *memory);
void allCaseSize(Memory *memory, numberOfSpaces number);

#endif
