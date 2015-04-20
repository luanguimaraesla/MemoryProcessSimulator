#include <stdio.h>
#include <stdlib.h>

#define SIZE_OF_ALL_SPACES 200

#define DESTRUCT_WITHOUT_MERGE  0
#define DESTRUCT_MERGE_NEXT     1
#define DESTRUCT_MERGE_PREV     2
#define DESTRUCT_MERGE_BOTH     3


/*-------------------------TYPEDEFS---------------------------*/

typedef int space;
typedef int numberOfSpaces;
typedef int destructType;

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
MemoryCase *addProcess(numberOfSpaces size, Memory *memory);
MemoryCase *allocProcessCase(numberOfSpaces size, MemoryCase *holeCaseThatFits);
MemoryCase *overwriteHole(MemoryCase *holeCaseThatFits);

MemoryCase * findNextHoleCase(MemoryCase *processCase);
MemoryCase * findPrevHoleCase(MemoryCase *processCase);

MemoryCase *nullMemoryCase(void);
MemoryCase *makeInitialMemoryCase(void);
MemoryCase *newMemoryCase(void);
MemoryCase *createAProcessCase(space begin, numberOfSpaces toUse);
MemoryCase *endProcess(MemoryCase *processCase, Memory *memory);
MemoryCase *mergeHoleCases(MemoryCase *holeCaseA, MemoryCase *holeCaseB, Memory *memory);
MemoryCase *destructWithoutMerge(MemoryCase *processCase, MemoryCase *prevHoleCase, 
				  MemoryCase *nextHoleCase, Memory *memory);


Process *initProcess(space begin, numberOfSpaces inUse);
Process *newProcess(void);

Hole *makeHole(space begin, numberOfSpaces available, MemoryCase *prevHole, MemoryCase *nextHole);
Hole *makeInitialHole(void);
Hole *nullHole(void);
Hole *newHole(void);

destructType selectDestructType(MemoryCase *processCase, MemoryCase *prevHoleCase, 
							 MemoryCase *nextHoleCase);
void printMemory(MemoryCase *firstCase);

/*---------------------------MAIN-----------------------------*/

int main(void){
	Memory *memory;
	MemoryCase *removeeAux;
	MemoryCase *removeeAux2;
	MemoryCase *removeeAux3;
	MemoryCase *removeeAux4;
	MemoryCase *removeeAux5;	

	memory = initMemory();
	
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);

	removeeAux3 = addProcess(10, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);

	endProcess(removeeAux3, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);
	
	removeeAux4 = addProcess(20, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);
	
	removeeAux = addProcess(50, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);

	removeeAux2 = addProcess(70, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);

	endProcess(removeeAux, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);
	
	removeeAux5 = addProcess(10, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);

	endProcess(removeeAux2, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);

	endProcess(removeeAux3, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);

	endProcess(removeeAux4, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);

	endProcess(removeeAux5, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);

	removeeAux2 = addProcess(70, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);

	endProcess(removeeAux2, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);

	removeeAux2 = addProcess(200, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
	printMemory(memory->begin);

	endProcess(removeeAux2, memory);
	printf("---------------------NOVA IMPRESSÃO---------------------");
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
		currentHoleCase = ((Hole *)(currentHoleCase->holeOrProcess))->nextHole;
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

MemoryCase * findNextHoleCase(MemoryCase *processCase){
	if (processCase == nullMemoryCase()) return nullMemoryCase();	
	else if(processCase->type == hole) return processCase;
	else return findNextHoleCase(processCase->next);
	
}

MemoryCase * findPrevHoleCase(MemoryCase *processCase){
	if (processCase == nullMemoryCase()) return nullMemoryCase();	
	else if(processCase->type == hole) return processCase;
	else return findPrevHoleCase(processCase->prev);
}

destructType selectDestructType(MemoryCase *processCase, 
				MemoryCase *prevHoleCase, 
				MemoryCase *nextHoleCase)
{	
	/*******************************
	 * DESTRUCT_WITHOUT_MERGE -> 0 *
	 * DESTRUCT_MERGE_NEXT    -> 1 *
	 * DESTRUCT_MERGE_PREV    -> 2 *
	 * DESTRUCT_MERGE_BOTH    -> 3 *
	 ******************************/

	if (!nextHoleCase && !prevHoleCase)
		return DESTRUCT_WITHOUT_MERGE;
	else if (nextHoleCase == processCase->next && prevHoleCase == processCase->prev 
		 && processCase->prev && processCase->next)
		return DESTRUCT_MERGE_BOTH;
	else if(nextHoleCase == processCase->next && processCase->next)
		return DESTRUCT_MERGE_NEXT;
	else if(prevHoleCase == processCase->prev && processCase->prev)
		return DESTRUCT_MERGE_PREV;
	else return DESTRUCT_WITHOUT_MERGE;
}

MemoryCase * endProcess(MemoryCase *processCase, Memory *memory){
	MemoryCase *nextHoleCase, *prevHoleCase, *newHoleCase;
	destructType option;
	
	if(processCase->type != process)
		return nullMemoryCase();
	
	nextHoleCase = findNextHoleCase(processCase);
	prevHoleCase = findPrevHoleCase(processCase);
	

	option = selectDestructType(processCase, prevHoleCase, nextHoleCase);	

	newHoleCase = destructWithoutMerge(processCase, prevHoleCase, nextHoleCase, memory);

	switch(option){
		case DESTRUCT_WITHOUT_MERGE:
			return newHoleCase;
		case DESTRUCT_MERGE_NEXT:
			return mergeHoleCases(newHoleCase, nextHoleCase, memory);
		case DESTRUCT_MERGE_PREV:
			return mergeHoleCases(newHoleCase, prevHoleCase, memory);
		case DESTRUCT_MERGE_BOTH:
			newHoleCase = mergeHoleCases(newHoleCase, prevHoleCase, memory);
			return mergeHoleCases(newHoleCase, nextHoleCase, memory);
		default:
			exit(1);
	}
}

MemoryCase * destructWithoutMerge(MemoryCase *processCase, MemoryCase *prevHoleCase, 
				  MemoryCase *nextHoleCase, Memory *memory){
	Hole *aHole;
	printf("\n\n%p\n\n" , (void *) nextHoleCase);
	processCase->type = hole;
	aHole = makeHole( ((Process *)(processCase->holeOrProcess))->begin, 
			    ((Process *)(processCase->holeOrProcess))->inUse,
			    prevHoleCase, nextHoleCase);
	processCase->holeOrProcess = (void *) aHole;

	if (nextHoleCase)
		((Hole *)(nextHoleCase->holeOrProcess))->prevHole = processCase;	
	else memory->lastHole = processCase;
	if (prevHoleCase)
		((Hole *)(prevHoleCase->holeOrProcess))->nextHole = processCase;
	else memory->firstHole = processCase; 

	return processCase;
}

MemoryCase * mergeHoleCases(MemoryCase *holeCaseA, MemoryCase *holeCaseB, Memory *memory){
	
	MemoryCase *holeCaseAux;
	if(holeCaseB->next == holeCaseA){
		holeCaseAux = holeCaseA;
		holeCaseA = holeCaseB;
		holeCaseB = holeCaseAux;
	}

	((Hole *)(holeCaseB->holeOrProcess))->begin = 
	((Hole *)(holeCaseA->holeOrProcess))->begin;

	((Hole *)(holeCaseB->holeOrProcess))->available += 
	((Hole *)(holeCaseA->holeOrProcess))->available;

	((Hole *)(holeCaseB->holeOrProcess))->prevHole = 
	((Hole *)(holeCaseA->holeOrProcess))->prevHole;

	if(((Hole *)(holeCaseA->holeOrProcess))->prevHole)
		((Hole *)(((Hole *)(holeCaseA->holeOrProcess))->prevHole->holeOrProcess))->nextHole =
											holeCaseB;
	
	holeCaseB->prev = holeCaseA->prev;
	if(holeCaseA->prev)
		holeCaseA->prev->next = holeCaseB;

	if (memory->begin == holeCaseA)
		memory->begin = holeCaseB;

	free(holeCaseA); 
	
	return holeCaseB;
}

/*-------------------MEMORY CASE FUNCTIONS--------------------*/

MemoryCase * makeInitialMemoryCase(void){
	MemoryCase *newMemoCase;
	newMemoCase = newMemoryCase();
	printf("\n\n%p\n\n" , (void *) newMemoCase);
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



/*
to do

make a findHoleThatFits function with a timer to limit the unnecessary system wait for a ideal case size


*/

















