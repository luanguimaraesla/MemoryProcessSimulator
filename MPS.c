#include <stdio.h>
#include <stdlib.h>

#define SIZE_OF_ALL_SPACES 200
#define HOLE 0
#define PROCESS 1

/*-------------------------TYPEDEFS---------------------------*/

typedef int space;
typedef int numberOfSpaces;

/*---------------------------ENUMS----------------------------*/

enum caseType{
	hole, process
};

typedef enum caseType caseType;

/*--------------------------STRUCTS---------------------------*/

struct MemoryCase{
	caseType type;

	void *holeOrProcess;

	struct MemoryCase *next;
	struct MemoryCase *prev;
};

struct Hole{
	space begin;
	numberOfSpaces available;

	struct MemoryCase *nextHole;
	struct MemoryCase *prevHole;
};

struct Process{
	space begin;
	numberOfSpaces inUse;
};

struct Memory{
	struct MemoryCase *begin;
	struct MemoryCase *end;
	struct MemoryCase *firstHole;
	struct MemoryCase *lastHole;
};

typedef struct MemoryCase MemoryCase;
typedef struct Hole Hole;
typedef struct Process Process;
typedef struct Memory Memory;

/*--------------------FUNCTIONS HEADERS-----------------------*/

Memory * initMemory(void);
MemoryCase *findHoleThatFits(numberOfSpaces size, Memory *memory);
MemoryCase * addProcess(numberOfSpaces size, Memory *memory);
MemoryCase *allocProcessCase(numberOfSpaces size, MemoryCase *holeCaseThatFits);
MemoryCase * overwriteHole(MemoryCase *holeCaseThatFits);

MemoryCase *nullMemoryCase(void);
MemoryCase *makeInitialMemoryCase(void);
MemoryCase *newMemoryCase(void);
MemoryCase *createAProcessCase(space begin, numberOfSpaces toUse);


Process *initProcess(space begin, numberOfSpaces inUse);
Process *newProcess(void);

Hole *makeHole(space begin, numberOfSpaces available,MemoryCase *prevHole, MemoryCase *nextHole);
Hole *makeInitialHole(void);
Hole *nullHole(void);
Hole *newHole(void);

void printMemory(MemoryCase *firstCase);

/*---------------------------MAIN-----------------------------*/

int main(void){
	Memory *memory;
	memory = initMemory();
	
	printMemory(memory->begin);
	addProcess(10, memory);
	printMemory(memory->begin);
	addProcess(20, memory);
	printMemory(memory->begin);
	addProcess(50, memory);
	printMemory(memory->begin);
	addProcess(70, memory);
	printMemory(memory->begin);
	addProcess(110, memory);
	printMemory(memory->begin);
	addProcess(10, memory);
	printMemory(memory->begin);

	return 0;
}

/*---------------------------MENU-----------------------------*/

/*----------------------PRINT FUNCTIONS-----------------------*/

void printMemory(MemoryCase *firstCase){
	if(firstCase){
		if(firstCase->type == hole){
			printf("\n----> HOLE\n");
			printf("Size: %d\n", ((Hole *)(firstCase->holeOrProcess))->available);
			printf("Begin: %d\n", ((Hole *)(firstCase->holeOrProcess))->begin);
		}
		else{
			printf("\n----> PROCESS\n");
			printf("Size: %d\n", ((Process *)(firstCase->holeOrProcess))->inUse);
			printf("Begin: %d\n", ((Process *)(firstCase->holeOrProcess))->begin);
		}
		printMemory(firstCase->next);
	}
}

/*---------------------MEMORY FUNCTIONS-----------------------*/

Memory * initMemory(void){
	Memory *newMemory;
	MemoryCase *newMemoryCase;	

	newMemory = (Memory *) malloc (sizeof(Memory));
	newMemoryCase = makeInitialMemoryCase();

	newMemory->begin = newMemoryCase;
	newMemory->end = newMemoryCase;
	newMemory->firstHole = newMemoryCase;
	newMemory->lastHole = newMemoryCase;

	return newMemory;
}

MemoryCase * addProcess(numberOfSpaces size, Memory *memory){	
	MemoryCase *holeCaseThatFits;
	MemoryCase *allocResponse;
	Hole *holeThatFits;	
	
	holeCaseThatFits = findHoleThatFits(size, memory);
	if(!holeCaseThatFits)
		return nullMemoryCase();
	
	holeThatFits = (Hole *) holeCaseThatFits->holeOrProcess;
	
	if(!((holeThatFits->available) - size)){
		if(holeCaseThatFits == memory->firstHole)
			memory->firstHole = holeThatFits->nextHole;
		else if(holeCaseThatFits == memory->lastHole)
			memory->lastHole = holeThatFits->prevHole;
		return overwriteHole(holeCaseThatFits);		
	}
	else{
		allocResponse = allocProcessCase(size, holeCaseThatFits);
		if(holeCaseThatFits == memory->begin)
			memory->begin = allocResponse;
		return allocResponse;
	}	
}

MemoryCase *findHoleThatFits(numberOfSpaces size, Memory *memory){	
	MemoryCase *smallerThatFits;
	MemoryCase *currentHoleCase = memory->firstHole;	
	Hole *currentHole, *smallerHole;
	currentHole = (Hole *) memory->firstHole->holeOrProcess;
				
	while(currentHoleCase && currentHole->available < size){
		if (currentHole->nextHole)
			currentHole = (Hole *) currentHole->nextHole->holeOrProcess;
		currentHoleCase = ((Hole *)(currentHoleCase->holeOrProcess))->nextHole;
	}
	if(!currentHoleCase) {		
		return nullMemoryCase();
		
	}

	smallerThatFits = currentHoleCase;
	smallerHole = currentHole;
	while(currentHoleCase){
		currentHole = (Hole *) currentHoleCase->holeOrProcess;
		if (currentHole->available >= 	     size 	&& 
		    currentHole->available < smallerHole->available){
			smallerHole = currentHole;
			smallerThatFits = currentHoleCase;
		}
		currentHoleCase = ((Hole *)(smallerThatFits->holeOrProcess))->nextHole;
	}

	return smallerThatFits;
}

MemoryCase *allocProcessCase(numberOfSpaces size, MemoryCase *holeCaseThatFits){
	MemoryCase *processCase;
	Hole *theHole;

	theHole =  (Hole *) holeCaseThatFits->holeOrProcess;
	processCase = createAProcessCase(theHole->begin, size);

	(theHole->available) -= size;
	(theHole->begin) += size;

	processCase->next = holeCaseThatFits;
	processCase->prev = holeCaseThatFits->prev;
	if(processCase->prev)
		processCase->prev->next = processCase;	
	holeCaseThatFits->prev = processCase;
	
	
	return processCase; 
}

MemoryCase * overwriteHole(MemoryCase *holeCaseThatFits){
	Hole *theHole;
	theHole =  (Hole *) holeCaseThatFits->holeOrProcess;	
	
	((Hole *)(theHole->prevHole->holeOrProcess))->nextHole = theHole->nextHole;
	((Hole *)(theHole->nextHole->holeOrProcess))->prevHole = theHole->prevHole;	

	holeCaseThatFits->type = process;
	holeCaseThatFits->holeOrProcess = (void *) initProcess(theHole->begin, theHole->available);

	return holeCaseThatFits;
}

/*-------------------MEMORY CASE FUNCTIONS--------------------*/

MemoryCase * makeInitialMemoryCase(void){
	MemoryCase *newMemoCase;
	newMemoCase = newMemoryCase();
	newMemoCase->type = hole;
	newMemoCase->holeOrProcess = (void *) makeInitialHole();
	newMemoCase->next = nullMemoryCase();
	newMemoCase->prev = nullMemoryCase();

	return newMemoCase;
}

MemoryCase * createAProcessCase(space begin, numberOfSpaces toUse){
	MemoryCase *newProcessCase;
	newProcessCase = newMemoryCase();
	newProcessCase->type = process;
	newProcessCase->next = nullMemoryCase();
	newProcessCase->prev = nullMemoryCase();
	newProcessCase->holeOrProcess = (void *) initProcess(begin, toUse);

	return newProcessCase;
}

MemoryCase *nullMemoryCase(void){
	return NULL;
}

MemoryCase *newMemoryCase(void){
	return (MemoryCase *) malloc (sizeof(MemoryCase));
}

/*---------------------PROCESS FUNCTIONS----------------------*/

Process *initProcess(space begin, numberOfSpaces inUse){
	Process * aProcess;
	aProcess = newProcess();
	aProcess->begin = begin;
	aProcess->inUse = inUse;

	return aProcess;
}

Process *newProcess(void){
	return (Process *) malloc (sizeof(Process));
}

/*-----------------------HOLE FUNCTIONS-----------------------*/

Hole *makeHole(space begin, numberOfSpaces available,MemoryCase *prevHole, MemoryCase *nextHole){
	Hole * aHole;
	aHole = newHole();
	aHole->begin = begin;
	aHole->available = available;
	aHole->prevHole = prevHole;
	aHole->nextHole = nextHole;

	return aHole;
}

Hole *makeInitialHole(void){
	return makeHole(0, SIZE_OF_ALL_SPACES, nullMemoryCase(), nullMemoryCase());
}

Hole *nullHole(void){
	return NULL;
}

Hole *newHole(void){
	return (Hole *) malloc (sizeof(Hole));
}

/*------------------------AUX FUNCTIONS-----------------------*/





















