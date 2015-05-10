#ifndef MMS_CREATION_FUNCTIONS
#define MMS_CREATION_FUNCTIONS
#include "mms_structures.h"

Memory * allocMemory(void);
MemoryCase * allocMemoryCase(void);
Hole * allocHole(void);
Process * allocProcess(void);
Memory * createMemory(numberOfSpaces size);
Memory *createAndSetMemory(numberOfSpaces available, numberOfSpaces inUse, numberOfProcesses running,
			   numberOfProcesses total, MemoryCase *begin, MemoryCase *firstProcessCase,
			   MemoryCase *firstHoleCase);
MemoryCase * createMemoryCase(memoryCaseType type, space begin, numberOfSpaces size,MemoryCase *next, MemoryCase *prev);
MemoryCase * createHoleCase(space begin, numberOfSpaces size, MemoryCase *nextHoleCase,
							MemoryCase *prevHoleCase, MemoryCase *next, MemoryCase *prev);
MemoryCase * createProcessCase(processID id, priority index, space begin, numberOfSpaces size,
							   MemoryCase *nextProcessCase, MemoryCase *prevProcessCase,
							   MemoryCase *next, MemoryCase *prev);
Hole * createHole(MemoryCase *nextHoleCase, MemoryCase *prevHoleCase);
Process * createProcess(processID id, priority index, MemoryCase *nextProcessCase, MemoryCase *prevProcessCase);
MemoryCase * createInitialHoleCase(numberOfSpaces size);
MemoryCase * nullMemoryCase(void);
processID getNewProcessID(void);

#endif
