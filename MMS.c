/* Memory Manager Simulator */

#include <stdio.h>
#include <stdlib.h>

/*------------------------TYPEDEFS-------------------------*/

typedef unsigned long time;
typedef unsigned long space;
typedef unsigned long numberOfSpaces;
typedef unsigned long processID;
typedef unsigned long numberOfProcesses;

/*--------------------------ENUMS--------------------------*/

enum memoryCaseType{
	hole, process
};

typedef enum memoryCaseType memoryCaseType;

/*-----------------------STRUCTS---------------------------*/

struct MemoryCase{
	memoryCaseType type;	
	void *holeOrProcess;
	space begin;
	numberOfSpaces size;
	struct MemoryCase *next;
	struct MemoryCase *prev;
};

struct Hole{
	struct MemoryCase *nextHoleCase;
	struct MemoryCase *prevHoleCase;
};

struct Process{
	processID id;
	time finalTime;
	struct MemoryCase *nextProcessCase;
	struct MemoryCase *prevProcessCase;
};

struct Memory{
	numberOfProcesses total;
	numberOfProcesses running;
	numberOfSpaces available;
	numberOfSpaces inUse;
	struct MemoryCase *begin;
	struct MemoryCase *firstProcessCase;
	struct MemoryCase *firstHoleCase;
};

typedef struct MemoryCase MemoryCase;
typedef struct Process Process;
typedef struct Hole Hole;
typedef struct Memory Memory;

/*-------------------FUNCTIONS HEADERS---------------------*/

/*                  1. Alloc functions                     */

Memory * allocMemory(void);
MemoryCase * allocMemoryCase(void);
Hole * allocHole(void);
Process * allocProcess(void);

/*				   2. Create functions                     */

Memory * createMemory(numberOfSpaces size);
MemoryCase * createMemoryCase(memoryCaseType type, space begin, numberOfSpaces size,
							  MemoryCase *next, MemoryCase *prev);
MemoryCase * createHoleCase(space begin, numberOfSpaces size, MemoryCase *nextHoleCase,
							MemoryCase *prevHoleCase, MemoryCase *next, MemoryCase *prev);
MemoryCase * createProcessCase(processID id, time finalTime, space begin, numberOfSpaces size,
							   MemoryCase *nextProcessCase, MemoryCase *prevProcessCase,
							   MemoryCase *next, MemoryCase *prev);
Hole * createHole(MemoryCase *nextHoleCase, MemoryCase *prevHoleCase);
Process * createProcess(processID id, time finalTime, 
						MemoryCase *nextProcessCase, 
						MemoryCase *prevProcessCase);
MemoryCase * createInitialHoleCase(numberOfSpaces size);

/*				     3. Null functions                     */

MemoryCase * nullMemoryCase(void);


/*-------------------------MAIN----------------------------*/

int main(void){
	
	return 0;
}

/*-------------------MEMORY FUNCTIONS----------------------*/

Memory * allocMemory(void){
	return (Memory *) malloc (sizeof(Memory));
}

Memory * createMemory(numberOfSpaces size){
	Memory *newMemory;

	newMemory = allocMemory();
	newMemory->available = size;
	newMemory->inUse = 0;
	newMemory->running = 0;
	newMemory->total = 0;
	newMemory->begin = createInitialHoleCase(size);
	newMemory->firstProcessCase = nullMemoryCase();
	newMemory->firstHoleCase = newMemory->begin;
	
	return newMemory;
}

/*----------------MEMORY CASE FUNCTIONS--------------------*/

MemoryCase * allocMemoryCase(void){
	return (MemoryCase *) malloc (sizeof(MemoryCase));
}

MemoryCase * createMemoryCase(memoryCaseType type, space begin, numberOfSpaces size,
							  MemoryCase *next, MemoryCase *prev){	
	MemoryCase *newMemoryCase;

	newMemoryCase = allocMemoryCase();
	newMemoryCase->type = type;
	newMemoryCase->begin = begin;
	newMemoryCase->size = size;
	newMemoryCase->next = next;
	newMemoryCase->prev = prev;

	return newMemoryCase;
}

MemoryCase * createProcessCase(processID id, time finalTime, space begin, numberOfSpaces size,
							   MemoryCase *nextProcessCase, MemoryCase *prevProcessCase,
							   MemoryCase *next, MemoryCase *prev){
	MemoryCase *newProcessCase;
	newProcessCase = createMemoryCase(process, begin, size, next, prev);
	newProcessCase->holeOrProcess = (void *) createProcess(id, finalTime, nextProcessCase, prevProcessCase);

	return newProcessCase;
}

MemoryCase * createHoleCase(space begin, numberOfSpaces size, MemoryCase *nextHoleCase,
							MemoryCase *prevHoleCase, MemoryCase *next, MemoryCase *prev){
	MemoryCase *newHoleCase;
	newHoleCase = createMemoryCase(hole, begin, size, next, prev);
	newHoleCase->holeOrProcess = (void *) createHole(nextHoleCase, prevHoleCase);
	return newHoleCase;
}

MemoryCase * createInitialHoleCase(numberOfSpaces size){
	MemoryCase * initialHoleCase;
	initialHoleCase = createHoleCase(0, size, nullMemoryCase(), nullMemoryCase(), 
											  nullMemoryCase(), nullMemoryCase());
	initialHoleCase->next = initialHoleCase;
	initialHoleCase->prev = initialHoleCase;
	((Hole *)(initialHoleCase->holeOrProcess))->nextHoleCase = initialHoleCase;
	((Hole *)(initialHoleCase->holeOrProcess))->prevHoleCase = initialHoleCase;
	return initialHoleCase;
}

MemoryCase * nullMemoryCase(void){
	return NULL;
}

/*-------------------HOLE FUNCTIONS------------------------*/

Hole * allocHole(void){
	return (Hole *) malloc (sizeof(Hole));
}

Hole * createHole(MemoryCase *nextHoleCase, MemoryCase *prevHoleCase){
	Hole * newHole;
	
	newHole = allocHole();
	newHole->nextHoleCase = nextHoleCase;
	newHole->prevHoleCase = prevHoleCase;
	return newHole;
}

/*-----------------PROCESS FUNCTIONS-----------------------*/

Process * allocProcess(void){
	return (Process *) malloc (sizeof(Process));
}

Process * createProcess(processID id, time finalTime, 
						MemoryCase *nextProcessCase, 
						MemoryCase *prevProcessCase){
	Process *newProcess;

	newProcess = allocProcess();
	newProcess->id = id;
	newProcess->finalTime = finalTime;
	newProcess->nextProcessCase = nextProcessCase;
	newProcess->prevProcessCase = prevProcessCase;

	return newProcess;
}









