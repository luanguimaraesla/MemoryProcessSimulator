#ifndef MMS_BEST_FIT
#define MMS_BEST_FIT
#include "mms_structures.h"

MemoryCase *reallocAndInsert_best(numberOfSpaces size, priority index, MemoryCase *insertBegin, Memory *memory);
MemoryCase *findTheBestFitHoleCase(numberOfSpaces size, Memory *memory);

#endif
