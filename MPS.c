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

	struct Hole *nextHole;
	struct Hole *prevHole;
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

MemoryCase * makeInitialMemoryCase(void);

Process *initProcess(space begin, numberOfSpaces inUse);

Hole *makeHole(space begin, numberOfSpaces available, Hole *prevHole, Hole *nextHole);
Hole *makeInitialHole(void);

/*---------------------------MAIN-----------------------------*/

int main(void){
	Memory *memory;

	memory = initMemory();

	return 0;
}

/*---------------------------MENU-----------------------------*/

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

/*-------------------MEMORY CASE FUNCTIONS--------------------*/

MemoryCase * makeInitialMemoryCase(void){
	MemoryCase *newMemoryCase;
	newMemoryCase = (MemoryCase *) malloc (sizeof(MemoryCase));
	newMemoryCase->type = hole;
	newMemoryCase->holeOrProcess = (void *) makeInitialHole();
	newMemoryCase->next = NULL;
	newMemoryCase->prev = NULL;

	return newMemoryCase;
}

/*---------------------PROCESS FUNCTIONS----------------------*/

Process *initProcess(space begin, numberOfSpaces inUse){
	Process * newProcess;
	newProcess = (Process *) malloc (sizeof(Process));
	newProcess->begin = begin;
	newProcess->inUse = inUse;

	return newProcess;
}

/*-----------------------HOLE FUNCTIONS-----------------------*/

Hole *makeHole(space begin, numberOfSpaces available,Hole *prevHole, Hole *nextHole){
	Hole * newHole;
	newHole = (Hole *) malloc (sizeof(Hole));
	newHole->begin = begin;
	newHole->available = available;
	newHole->prevHole = prevHole;
	newHole->nextHole = nextHole;

	return newHole;
}

Hole *makeInitialHole(void){
	return makeHole(0, SIZE_OF_ALL_SPACES, NULL, NULL);
}
