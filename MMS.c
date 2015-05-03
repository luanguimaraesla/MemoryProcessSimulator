/* Memory Manager Simulator */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DESTRUCT_WITHOUT_MERGE  0
#define DESTRUCT_MERGE_NEXT     1
#define DESTRUCT_MERGE_PREV     2
#define DESTRUCT_MERGE_BOTH     3

/*------------------------TYPEDEFS-------------------------*/

typedef unsigned long time;
typedef unsigned long space;
typedef unsigned long numberOfSpaces;
typedef unsigned long processID;
typedef unsigned long numberOfProcesses;
typedef int destructType;
typedef int insertionMode;

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

/*--------------------GLOBAL VARIABLES---------------------*/

time programTime = 0;

/*-------------------FUNCTIONS HEADERS---------------------*/

/*1. Alloc functions*/

Memory * allocMemory(void);
MemoryCase * allocMemoryCase(void);
Hole * allocHole(void);
Process * allocProcess(void);

/*2. Create functions*/

Memory * createMemory(numberOfSpaces size);
MemoryCase * createMemoryCase(memoryCaseType type, space begin, numberOfSpaces size,MemoryCase *next, MemoryCase *prev);
MemoryCase * createHoleCase(space begin, numberOfSpaces size, MemoryCase *nextHoleCase,
							MemoryCase *prevHoleCase, MemoryCase *next, MemoryCase *prev);
MemoryCase * createProcessCase(processID id, time finalTime, space begin, numberOfSpaces size,
							   MemoryCase *nextProcessCase, MemoryCase *prevProcessCase,
							   MemoryCase *next, MemoryCase *prev);
Hole * createHole(MemoryCase *nextHoleCase, MemoryCase *prevHoleCase);
Process * createProcess(processID id, time finalTime, MemoryCase *nextProcessCase, MemoryCase *prevProcessCase);
MemoryCase * createInitialHoleCase(numberOfSpaces size);

/*3. Null functions*/

MemoryCase * nullMemoryCase(void);

/*4. Auxiliar functions*/

processID getNewProcessID(void);

/*5. Add process functions*/

MemoryCase *addProcessFirstFit(numberOfSpaces size, time executionTime, Memory *memory);
MemoryCase *reallocAndInsert_best(numberOfSpaces size, time executionTime, MemoryCase *insertBegin, Memory *memory);
void removeHoleCase(MemoryCase *toRemove, Memory *memory);
MemoryCase *overwriteHoleCase(time executionTime, MemoryCase *holeToOverwrite, Memory *memory);
MemoryCase *findNextTimeListProcessCase(time finalTime, MemoryCase *firstProcessCase);
MemoryCase *divideAndInsert(numberOfSpaces size, time executionTime, MemoryCase * holeCaseToDivide, Memory *memory);
MemoryCase *findTheBestFitHoleCase(numberOfSpaces size, Memory *memory);
MemoryCase *reallocAndInsert_worst(numberOfSpaces size, time executionTime, MemoryCase *insertBegin, Memory *memory);
MemoryCase *findTheWorstFitHoleCase(numberOfSpaces size, Memory *memory);
MemoryCase * addProcessWorstFit(numberOfSpaces size, time executionTime, Memory *memory);
MemoryCase * addProcessBestFit(numberOfSpaces size, time executionTime, Memory *memory);

/*6. Print functions*/

void printMemory(MemoryCase *firstCase);
void printProcessList(MemoryCase *firstProcessCase);
void printHoleList(MemoryCase *firstHoleCase);

/*7. End process functions*/

MemoryCase * endProcess(MemoryCase *processCase, Memory *memory);
destructType selectDestructType(MemoryCase *processCase, MemoryCase *prevHoleCase, MemoryCase *nextHoleCase);
MemoryCase * findPrevHoleCase(MemoryCase *processCase, Memory *memory);
MemoryCase * findNextHoleCase(MemoryCase *processCase, Memory* memory);
MemoryCase * destructWithoutMerge(MemoryCase *processCase, MemoryCase *prevHoleCase, MemoryCase *nextHoleCase, Memory *memory);
MemoryCase * removeProcessOfProcessList(MemoryCase *processCase, Memory *memory);
MemoryCase * mergeHoleCases(MemoryCase *holeCaseA, MemoryCase *holeCaseB, Memory *memory);

/*8. Menu functions*/
void printInsertionModeSelectionMenu(void);
insertionMode getInsertionMode(void);
void printMemorySizeMenu(void);
numberOfSpaces getMemorySize(void);


/*-------------------------MAIN----------------------------*/

int main(void){

	Memory *memory;
	MemoryCase *(*addProcess)();
	MemoryCase *test[3];
	memory = createMemory(getMemorySize());
	
	printf("Criada uma memoria de tamanho: %lu\n", memory->available);
	switch(getInsertionMode()){
		case 1:
			addProcess = addProcessFirstFit;
			break;
		case 2:
			addProcess = addProcessWorstFit;
			break;
		case 3:
			addProcess = addProcessBestFit;
			break;
		default:
			printf("\nERROR!\n");
			exit(1); 
	}

	test[0] = (*addProcess)(10, 30, memory);
	test[1] =(*addProcess)(20, 50, memory);
	test[2] =(*addProcess)(50, 20, memory);
	endProcess(test[1], memory);
	test[1] = (*addProcess)(60, 40, memory);
	
	printf("\n--------------MEMORY STATUS-------------");
	printMemory(memory->begin);

	return 0;
}

/*---------------------MENU FUNCTIONS----------------------*/

void printInsertionModeSelectionMenu(void){
	printf("\nSelect the insertion mode: ");
	printf("\n1. First-Fit.\n2. Worst-Fit\n3. Best-Fit\n\nSelect: ");
}

insertionMode getInsertionMode(void){
	insertionMode option;
	printInsertionModeSelectionMenu();
	do{	
		scanf("%d", &option);
	}while(option < 1 && option > 3);
	return option;
}

void printMemorySizeMenu(void){
	printf("\nType the memory size: ");
}

numberOfSpaces getMemorySize(void){
	numberOfSpaces size;
	printMemorySizeMenu();
	scanf("%lu", &size);
	return size;
}

/*--------------------PRINT FUNCTIONS----------------------*/

void printMemory(MemoryCase *firstCase){
	MemoryCase *firstPrintedCase = firstCase;
	do{
		if(firstCase->type == hole){
			printf("\n----> HOLE\n");
			printf("Size: %lu\n", firstCase->size);
			printf("Begin: %lu\n", firstCase->begin);
		}
		else{
			printf("\n----> PROCESS\n");
			printf("ID: %lu\n", ((Process *)(firstCase->holeOrProcess))->id);
			printf("Size: %lu\n", firstCase->size);
			printf("Begin: %lu\n", firstCase->begin);
		}
		firstCase = firstCase->next;
	}while(firstCase != firstPrintedCase);
}

void printProcessList(MemoryCase *firstProcessCase){
	if(firstProcessCase){	
		printf("\n----> PROCESS\n");
		printf("ID: %lu\n", ((Process *)(firstProcessCase->holeOrProcess))->id);
		printf("Size: %lu\n", firstProcessCase->size);
		printf("Begin: %lu\n", firstProcessCase->begin);
		printProcessList(((Process *)(firstProcessCase->holeOrProcess))->nextProcessCase);
	}
}

void printHoleList(MemoryCase *firstHoleCase){
	MemoryCase *firstHolePrinted = firstHoleCase;
	if (firstHoleCase){
		do{
			printf("\n----> HOLE\n");
			printf("Size: %lu\n", firstHoleCase->size);
			printf("Begin: %lu\n", firstHoleCase->begin);
			firstHoleCase = ((Hole *)(firstHoleCase->holeOrProcess))->nextHoleCase;
		}while(firstHoleCase != firstHolePrinted);
	}
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

/*WORST FIT INSERTION*/

MemoryCase *findTheWorstFitHoleCase(numberOfSpaces size, Memory *memory){
	MemoryCase * runner = memory->firstHoleCase;
	MemoryCase * worstFitHoleCase = runner;
	numberOfSpaces maxDiference = 0;
	do{
		if(abs(runner->size - size) > maxDiference){
			maxDiference = abs(runner->size - size);
			worstFitHoleCase = runner;
		}
		runner = ((Hole *)(runner->holeOrProcess))->nextHoleCase;
	}while(runner != memory->firstHoleCase);
	return worstFitHoleCase;	
}

MemoryCase *addProcessWorstFit(numberOfSpaces size, time executionTime, Memory *memory){
	MemoryCase *worstFitHoleCase = findTheWorstFitHoleCase(size, memory);

	if(memory->available < size || !worstFitHoleCase)
		return nullMemoryCase();
	if(size >= worstFitHoleCase->size)
		return reallocAndInsert_worst(size, executionTime, worstFitHoleCase, memory);
	else
		return divideAndInsert(size, executionTime, worstFitHoleCase, memory);
}

MemoryCase *reallocAndInsert_worst(numberOfSpaces size, time executionTime, MemoryCase *insertBegin, Memory *memory){
	MemoryCase *currentPrevHoleCase = ((Hole*)(insertBegin->holeOrProcess))->prevHoleCase;
	MemoryCase *currentNextHoleCase = ((Hole*)(insertBegin->holeOrProcess))->nextHoleCase;
	MemoryCase *runner;
	numberOfSpaces currentSizeAux = 0;
	numberOfSpaces currentPrevSize = 0;
	numberOfSpaces currentNextSize = 0;
	
	while(insertBegin->size + currentPrevSize + currentNextSize < size){
		if(currentPrevHoleCase->size < currentNextHoleCase->size){
			if(currentPrevHoleCase->size + currentNextSize + insertBegin->size + currentPrevSize > size)
				currentPrevSize += (size - currentNextSize - currentPrevSize - insertBegin->size);
			else
				currentPrevSize += currentNextHoleCase->size;
			currentPrevHoleCase = ((Hole *)(currentPrevHoleCase->holeOrProcess))->prevHoleCase;
		}
		else{
			if(currentNextHoleCase->size + currentNextSize + insertBegin->size + currentPrevSize > size)
				currentNextSize += (size - currentNextSize - currentPrevSize - insertBegin->size);
			else
				currentNextSize += currentNextHoleCase->size;
			currentNextHoleCase = ((Hole *)(currentNextHoleCase->holeOrProcess))->nextHoleCase;
		}
	}

	runner = insertBegin->next;
	while(runner != currentNextHoleCase && currentNextSize){
		if(runner->type == process){
			runner->begin += currentNextSize - currentSizeAux;
		}		
		else{
			if(currentNextSize > currentSizeAux + runner->size){
				currentSizeAux += runner->size;
				runner = runner->next;
				removeHoleCase(runner->prev, memory);
				continue;
			}
			else{
				runner->size -= currentNextSize - currentSizeAux;
				runner->begin += currentNextSize - currentSizeAux;
				if(runner->size == 0)
						removeHoleCase(runner, memory);
				break;
			}
		}
		runner = runner->next;
	}

	currentSizeAux = 0;
	runner = insertBegin->prev;
	while(runner != currentPrevHoleCase && currentPrevSize){
		if(runner->type == process)
			runner->begin -= (currentPrevSize - currentSizeAux);		
		else{
			if(currentPrevSize > currentSizeAux + runner->size){
				currentSizeAux += runner->size;
				runner = runner->prev;
				removeHoleCase(runner->next, memory);
				continue;
			}
			else{
				runner->size -= (currentPrevSize - currentSizeAux);
				if(runner->size == 0)
						removeHoleCase(runner, memory);
				break;
			}
		}
		runner = runner->prev;
	}
	
	insertBegin->size = size;
	insertBegin->begin = insertBegin->prev->size + insertBegin->prev->begin >= memory->inUse + memory->available ?
						insertBegin->prev->size + insertBegin->prev->begin - memory->inUse + memory->available :
						insertBegin->prev->size + insertBegin->prev->begin;

	return overwriteHoleCase(executionTime, insertBegin, memory);
}

/*FIRST FIT INSERTION*/

MemoryCase *reallocAndInsert_best(numberOfSpaces size, time executionTime, MemoryCase *insertBegin, Memory *memory){
	MemoryCase *currentPrevHoleCase = ((Hole*)(insertBegin->holeOrProcess))->prevHoleCase;
	MemoryCase *currentNextHoleCase = ((Hole*)(insertBegin->holeOrProcess))->nextHoleCase;
	MemoryCase *runner;
	numberOfSpaces currentSizeAux = 0;
	numberOfSpaces currentPrevSize = 0;
	numberOfSpaces currentNextSize = 0;
	
	while(insertBegin->size + currentPrevSize + currentNextSize < size){
		if(currentPrevHoleCase->size > currentNextHoleCase->size){
			if(currentPrevHoleCase->size + currentNextSize + insertBegin->size + currentPrevSize > size)
				currentPrevSize += (size - currentNextSize - currentPrevSize - insertBegin->size);
			else
				currentPrevSize += currentNextHoleCase->size;
			currentPrevHoleCase = ((Hole *)(currentPrevHoleCase->holeOrProcess))->prevHoleCase;
		}
		else{
			if(currentNextHoleCase->size + currentNextSize + insertBegin->size + currentPrevSize > size)
				currentNextSize += (size - currentNextSize - currentPrevSize - insertBegin->size);
			else
				currentNextSize += currentNextHoleCase->size;
			currentNextHoleCase = ((Hole *)(currentNextHoleCase->holeOrProcess))->nextHoleCase;
		}
	}

	runner = insertBegin->next;
	while(runner != currentNextHoleCase && currentNextSize){
		if(runner->type == process){
			runner->begin += currentNextSize - currentSizeAux;
		}		
		else{
			if(currentNextSize > currentSizeAux + runner->size){
				currentSizeAux += runner->size;
				runner = runner->next;
				removeHoleCase(runner->prev, memory);
				continue;
			}
			else{
				runner->size -= currentNextSize - currentSizeAux;
				runner->begin += currentNextSize - currentSizeAux;
				if(runner->size == 0)
						removeHoleCase(runner, memory);
				break;
			}
		}
		runner = runner->next;
	}

	currentSizeAux = 0;
	runner = insertBegin->prev;
	while(runner != currentPrevHoleCase && currentPrevSize){
		if(runner->type == process)
			runner->begin -= (currentPrevSize - currentSizeAux);		
		else{
			if(currentPrevSize > currentSizeAux + runner->size){
				currentSizeAux += runner->size;
				runner = runner->prev;
				removeHoleCase(runner->next, memory);
				continue;
			}
			else{
				runner->size -= (currentPrevSize - currentSizeAux);
				if(runner->size == 0)
						removeHoleCase(runner, memory);
				break;
			}
		}
		runner = runner->prev;
	}
	
	insertBegin->size = size;
	insertBegin->begin = insertBegin->prev->size + insertBegin->prev->begin >= memory->inUse + memory->available ?
						insertBegin->prev->size + insertBegin->prev->begin - memory->inUse + memory->available :
						insertBegin->prev->size + insertBegin->prev->begin;

	return overwriteHoleCase(executionTime, insertBegin, memory);
}

void removeHoleCase(MemoryCase *toRemove, Memory *memory){
	if(((Hole *)(toRemove->holeOrProcess))->nextHoleCase == toRemove)
		memory->firstHoleCase = nullMemoryCase();
	else if(memory->firstHoleCase == toRemove)
		memory->firstHoleCase = ((Hole *)(toRemove->holeOrProcess))->nextHoleCase;
	if(memory->begin == toRemove)
		memory->begin = memory->begin->next;
	toRemove->prev->next = toRemove->next;
	toRemove->next->prev = toRemove->prev;
	((Hole*)(((Hole*)(toRemove->holeOrProcess))->prevHoleCase->holeOrProcess))->nextHoleCase =
	((Hole*)(toRemove->holeOrProcess))->nextHoleCase;
	((Hole*)(((Hole*)(toRemove->holeOrProcess))->nextHoleCase->holeOrProcess))->prevHoleCase =
	((Hole*)(toRemove->holeOrProcess))->prevHoleCase;
	
	free(toRemove);
}		

MemoryCase *overwriteHoleCase(time executionTime, MemoryCase *holeToOverwrite, Memory *memory){
	MemoryCase *nextProcessCase;
	MemoryCase *prevProcessCase;

	if(((Hole *)(holeToOverwrite->holeOrProcess))->nextHoleCase == holeToOverwrite)
		memory->firstHoleCase = nullMemoryCase();
	else if(memory->firstHoleCase == holeToOverwrite)
		memory->firstHoleCase = ((Hole *)(holeToOverwrite->holeOrProcess))->nextHoleCase;
	
	((Hole*)(((Hole*)(holeToOverwrite->holeOrProcess))->prevHoleCase->holeOrProcess))->nextHoleCase =
	((Hole*)(holeToOverwrite->holeOrProcess))->nextHoleCase;
	((Hole*)(((Hole*)(holeToOverwrite->holeOrProcess))->nextHoleCase->holeOrProcess))->prevHoleCase =
	((Hole*)(holeToOverwrite->holeOrProcess))->prevHoleCase;

	nextProcessCase = findNextTimeListProcessCase(executionTime + programTime, memory->firstProcessCase);
	prevProcessCase = nextProcessCase ? ((Process *)(nextProcessCase->holeOrProcess))->prevProcessCase : nullMemoryCase();	
	holeToOverwrite->holeOrProcess = createProcess(getNewProcessID(), executionTime + programTime, nextProcessCase, prevProcessCase);
	
	if(memory->firstProcessCase == nullMemoryCase() || nextProcessCase == memory->firstProcessCase)
		memory->firstProcessCase = holeToOverwrite;
	if(nextProcessCase)
		((Process*)(nextProcessCase->holeOrProcess))->prevProcessCase = holeToOverwrite;
	if(prevProcessCase)
		((Process*)(prevProcessCase->holeOrProcess))->nextProcessCase = holeToOverwrite;

	holeToOverwrite->type = process;

	return holeToOverwrite;
}

MemoryCase *findNextTimeListProcessCase(time finalTime, MemoryCase *firstProcessCase){
	if(!firstProcessCase) return nullMemoryCase();
	else if (((Process *)(firstProcessCase->holeOrProcess))->finalTime >= finalTime) return firstProcessCase;
	else return findNextTimeListProcessCase(finalTime, ((Process *)(firstProcessCase->holeOrProcess))->nextProcessCase);
}

MemoryCase *addProcessFirstFit(numberOfSpaces size, time executionTime, Memory *memory){
	MemoryCase *firstHoleCase = memory->firstHoleCase;
	if(memory->available < size || !firstHoleCase)
		return nullMemoryCase();
	if(size >= firstHoleCase->size)
		return reallocAndInsert_best(size, executionTime, firstHoleCase, memory);
	else
		return divideAndInsert(size, executionTime, firstHoleCase, memory);
}

/*BEST FIT INSERTION*/
MemoryCase *findTheBestFitHoleCase(numberOfSpaces size, Memory *memory){
	MemoryCase * runner = memory->firstHoleCase;
	MemoryCase * bestFitHoleCase = runner;
	numberOfSpaces minDiference = runner->size;
	do{
		if(abs(runner->size - size) < minDiference){
			minDiference = abs(runner->size - size);
			bestFitHoleCase = runner;
		}
		runner = ((Hole *)(runner->holeOrProcess))->nextHoleCase;
	}while(runner != memory->firstHoleCase);
	return bestFitHoleCase;	
}

MemoryCase *addProcessBestFit(numberOfSpaces size, time executionTime, Memory *memory){
	MemoryCase *bestFitHoleCase = findTheBestFitHoleCase(size, memory);

	if(memory->available < size || !bestFitHoleCase)
		return nullMemoryCase();
	if(size >= bestFitHoleCase->size)
		return reallocAndInsert_best(size, executionTime, bestFitHoleCase, memory);
	else
		return divideAndInsert(size, executionTime, bestFitHoleCase, memory);
}

MemoryCase *divideAndInsert(numberOfSpaces size, time executionTime, MemoryCase * holeCaseToDivide, Memory *memory){
	MemoryCase *newProcessCase;
	MemoryCase *nextProcessCase;
	MemoryCase *prevProcessCase;

	nextProcessCase = findNextTimeListProcessCase(executionTime + programTime, memory->firstProcessCase);
	prevProcessCase = nextProcessCase ? ((Process *)(nextProcessCase->holeOrProcess))->prevProcessCase : nullMemoryCase();
	newProcessCase = createProcessCase(getNewProcessID(), executionTime + programTime, holeCaseToDivide->begin, size,
					   nextProcessCase, prevProcessCase, holeCaseToDivide, holeCaseToDivide->prev);

	holeCaseToDivide->size -= size;
	holeCaseToDivide->begin += size;
	holeCaseToDivide->prev->next = newProcessCase;
	holeCaseToDivide->prev = newProcessCase;

	if(holeCaseToDivide == memory->begin)
		memory->begin = newProcessCase;
	if(nextProcessCase == memory->firstProcessCase)
		memory->firstProcessCase = newProcessCase;
	if(nextProcessCase)
		((Process*)(nextProcessCase->holeOrProcess))->prevProcessCase = newProcessCase;
	if(prevProcessCase)
		((Process*)(prevProcessCase->holeOrProcess))->nextProcessCase = newProcessCase;

	return newProcessCase;
}

MemoryCase * findNextHoleCase(MemoryCase *processCase, Memory* memory){
	MemoryCase *runner = processCase->next;
	do{
		if(runner->type == hole)
			return runner;
		runner = runner->next;
	}while(runner != processCase);
	return nullMemoryCase();
}

/*remove process functions*/

MemoryCase * findPrevHoleCase(MemoryCase *processCase, Memory *memory){
	MemoryCase *runner = processCase->prev;
	do{
		if(runner->type == hole)
			return runner;
		runner = runner->prev;
	}while(runner != processCase);
	return nullMemoryCase();
}

destructType selectDestructType(MemoryCase *processCase, 
								MemoryCase *prevHoleCase, 
								MemoryCase *nextHoleCase){	
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
	
	nextHoleCase = findNextHoleCase(processCase, memory);
	prevHoleCase = findPrevHoleCase(processCase, memory);
	
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
			if(nextHoleCase != prevHoleCase)
				newHoleCase = mergeHoleCases(newHoleCase, prevHoleCase, memory);
			return mergeHoleCases(newHoleCase, nextHoleCase, memory);
		default:
			exit(1);
	}
}

MemoryCase * destructWithoutMerge(MemoryCase *processCase, MemoryCase *prevHoleCase, 
				  MemoryCase *nextHoleCase, Memory *memory){
	Hole *aHole;
	MemoryCase *holeCase;

	holeCase = removeProcessOfProcessList(processCase, memory);	
	holeCase->type = hole;

	aHole = createHole(nextHoleCase, prevHoleCase);
	holeCase->holeOrProcess = (void *) aHole;

	if (nextHoleCase)
		((Hole *)(nextHoleCase->holeOrProcess))->prevHoleCase = holeCase;
	if (prevHoleCase)
		((Hole *)(prevHoleCase->holeOrProcess))->nextHoleCase = holeCase;

	if(memory->firstHoleCase == nextHoleCase)	
		memory->firstHoleCase = holeCase;

	return holeCase;
}

MemoryCase * removeProcessOfProcessList(MemoryCase *processCase, Memory *memory){

	if(((Process *)(processCase->holeOrProcess))->prevProcessCase)
		((Process*)(((Process *)(processCase->holeOrProcess))->prevProcessCase->holeOrProcess))->nextProcessCase = 
		((Process *)(processCase->holeOrProcess))->nextProcessCase;
	else memory->firstProcessCase = ((Process *)(processCase->holeOrProcess))->nextProcessCase;

	if(((Process *)(processCase->holeOrProcess))->nextProcessCase)
		((Process*)(((Process *)(processCase->holeOrProcess))->nextProcessCase->holeOrProcess))->prevProcessCase = 
		((Process *)(processCase->holeOrProcess))->prevProcessCase;

	return processCase;
}

MemoryCase * mergeHoleCases(MemoryCase *holeCaseA, MemoryCase *holeCaseB, Memory *memory){
	
	MemoryCase *holeCaseAux;
	if(holeCaseB->next == holeCaseA){
		holeCaseAux = holeCaseA;
		holeCaseA = holeCaseB;
		holeCaseB = holeCaseAux;
	}

	holeCaseB->begin = holeCaseA->begin;
	holeCaseB->size += holeCaseA->size;

	((Hole *)(holeCaseB->holeOrProcess))->prevHoleCase = 
	((Hole *)(holeCaseA->holeOrProcess))->prevHoleCase;

	if(((Hole *)(holeCaseA->holeOrProcess))->prevHoleCase)
		((Hole *)(((Hole *)(holeCaseA->holeOrProcess))->
		prevHoleCase->holeOrProcess))->nextHoleCase = holeCaseB;	

	holeCaseB->prev = holeCaseA->prev;

	if(holeCaseA->prev)
		holeCaseA->prev->next = holeCaseB;

	if (memory->begin == holeCaseA)
		memory->begin = holeCaseB;

	if (memory->firstHoleCase == holeCaseA)
		memory->firstHoleCase = holeCaseB;	

	free(holeCaseA);
	return holeCaseB;
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

/*----------------------AUXILIAR---------------------------*/

processID getNewProcessID(void){
	static processID id = -1;
	return ++id;
}








