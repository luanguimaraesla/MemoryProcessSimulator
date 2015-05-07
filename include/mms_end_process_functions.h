#ifndef MMS_END_FUNCTIONS
#define MMS_END_FUNCTIONS
#include "mms_structures.h"

#define DESTRUCT_WITHOUT_MERGE  0
#define DESTRUCT_MERGE_NEXT     1
#define DESTRUCT_MERGE_PREV     2
#define DESTRUCT_MERGE_BOTH     3

MemoryCase * endProcess(MemoryCase *processCase, Memory *memory);
destructType selectDestructType(MemoryCase *processCase, MemoryCase *prevHoleCase, MemoryCase *nextHoleCase);
MemoryCase * findPrevHoleCase(MemoryCase *processCase, Memory *memory);
MemoryCase * findNextHoleCase(MemoryCase *processCase, Memory* memory);
MemoryCase * destructWithoutMerge(MemoryCase *processCase, MemoryCase *prevHoleCase, MemoryCase *nextHoleCase, Memory *memory);
MemoryCase * removeProcessOfProcessList(MemoryCase *processCase, Memory *memory);
MemoryCase * mergeHoleCases(MemoryCase *holeCaseA, MemoryCase *holeCaseB, Memory *memory);

#endif
