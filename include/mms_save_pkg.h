#ifndef MMS_SAVE_PKG
#define MMS_SAVE_PKG

#include "mms_structures.h"
#include "mms_thread_args.h"
#include <stdio.h>

unsigned long getFileInt(FILE *arq);
char getFileChar(FILE *arq);
Memory *dropSimulationFromFile(FILE *arq, rcp_arg *keepOldArgs, insertionMode *functionID);
void sintetizeMemoryPointers(Memory *memory);
void sintetizeHoleCasePointers(Memory *memory);
void sintetizeProcessCasePointers(Memory *memory);
void bubbleSortForProcessPriority(Memory *memory);
void writeFileInt(unsigned long number, FILE *arq);
void writeFileChar(char letter, FILE *arq);
void pushMemoryToFile(FILE *arq, rcp_arg *keepCurrentArgs, insertionMode functionID);
unsigned int getNumberOfMemoryCases(Memory *memory);
void writeFileMemoryCases(Memory *memory, FILE *arq);

#endif
