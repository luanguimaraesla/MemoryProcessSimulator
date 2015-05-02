/* Memory Manager Simulator */

#include <stdio.h>
#include <stdlib.h>

/*------------------------TYPEDEFS-------------------------*/

typedef unsigned long time;
typedef unsigned long space;
typedef unsigned long numberOfSpaces;
typedef unsigned long processID;
typedef unsigned long numberOfProcesses;
typedef int destructType;

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

/*		           3. Null functions                     */

MemoryCase * nullMemoryCase(void);

/*                 4. Auxiliar functions                 */

processID getNewProcessID(void);

/*                 5. Add process functions                */

MemoryCase *addProcessFirstFit(numberOfSpaces size, time executionTime, Memory *memory);
MemoryCase *reallocAndInsert_best(numberOfSpaces size, time executionTime, MemoryCase *insertBegin, Memory *memory);
void removeHoleCase(MemoryCase *toRemove, Memory *memory);
MemoryCase *overwriteHoleCase(time executionTime, MemoryCase *holeToOverwrite, Memory *memory);
MemoryCase *findNextTimeListProcessCase(time finalTime, MemoryCase *firstProcessCase);
MemoryCase *divideAndInsert(numberOfSpaces size, time executionTime, MemoryCase * holeCaseToDivide, Memory *memory);

/*                 6. Print functions                    */
void printMemory(MemoryCase *firstCase);
void printProcessList(MemoryCase *firstProcessCase);

/*-------------------------MAIN----------------------------*/

int main(void){

	Memory *memory;
	memory = createMemory(300);
	
	printf("Criada uma memoria de tamanho: %lu\n", memory->available);
	addProcessFirstFit(200, 10, memory);
	addProcessFirstFit(20, 10, memory);
	addProcessFirstFit(40, 5, memory);
	printf("---------------------MEMORY LIST---->");
	printMemory(memory->begin);
	printf("--------------------PROCESS LIST---->");
	printProcessList(memory->firstProcessCase);

	return 0;
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

MemoryCase *reallocAndInsert_best(numberOfSpaces size, time executionTime, MemoryCase *insertBegin, Memory *memory){
	MemoryCase *currentPrevHoleCase = ((Hole*)(insertBegin->holeOrProcess))->prevHoleCase;
	MemoryCase *currentNextHoleCase = ((Hole*)(insertBegin->holeOrProcess))->nextHoleCase;
	MemoryCase *runner;
	numberOfSpaces currentSizeAux = 0;
	numberOfSpaces currentPrevSize = 0;
	numberOfSpaces currentNextSize = 0;
	
	
	while(insertBegin->size + currentPrevSize + currentNextSize < size){
		if(currentPrevHoleCase->size > currentNextHoleCase->size){
			currentPrevSize += currentPrevHoleCase->size;
			currentPrevHoleCase = ((Hole *)(currentPrevHoleCase->holeOrProcess))->prevHoleCase;
		}
		else{
			currentNextSize += currentNextHoleCase->size;
			currentNextHoleCase = ((Hole *)(currentNextHoleCase->holeOrProcess))->nextHoleCase;
		}
	}

	runner = insertBegin->next;
	while(runner != currentNextHoleCase && currentNextSize){
		if(runner->type == process)
			runner->begin += currentNextSize - currentSizeAux;		
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

MemoryCase *divideAndInsert(numberOfSpaces size, time executionTime, MemoryCase * holeCaseToDivide, Memory *memory){
	MemoryCase *newProcessCase;
	MemoryCase *nextProcessCase = findNextTimeListProcessCase(executionTime + programTime, memory->firstProcessCase);
	MemoryCase *prevProcessCase = nextProcessCase ? ((Process *)(nextProcessCase->holeOrProcess))->prevProcessCase : nullMemoryCase();
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
	}while(runner != memory->begin);
	return nullMemoryCase();
}

MemoryCase * findPrevHoleCase(MemoryCase *processCase){
	MemoryCase *runner = processCase->next;
	do{
		if(runner->type == hole)
			return runner;
		runner = runner->prev;
	}while(runner != memory->begin->prev);
	return nullMemoryCase();
}


MemoryCase *endProcess(){
	
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








