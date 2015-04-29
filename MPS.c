#include <stdio.h>
#include <stdlib.h>

#define SIZE_OF_ALL_SPACES 200

#define DESTRUCT_WITHOUT_MERGE  0
#define DESTRUCT_MERGE_NEXT     1
#define DESTRUCT_MERGE_PREV     2
#define DESTRUCT_MERGE_BOTH     3


/*-------------------------TYPEDEFS---------------------------*/

typedef unsigned long space;
typedef unsigned long numberOfSpaces;
typedef unsigned long processID;
typedef unsigned long counter;
typedef int destructType;
typedef int menuOption;


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
	processID ID;
	space begin;
	numberOfSpaces inUse;

	struct MemoryCase *nextProcess;
	struct MemoryCase *prevProcess;
};

struct EachProcessManager{
	processID ID;
	numberOfSpaces size;

	struct MemoryCase *head;
	struct MemoryCase *finish;

	struct EachProcessManager * prevProcessManager;
	struct EachProcessManager * nextProcessManager;
};

struct MasterProcessManager{
	counter processCounter;
	numberOfSpaces inUse;
	
	struct EachProcessManager * firstProcessManager;
	struct EachProcessManager * lastProcessManager;
};

struct Memory{
	struct MemoryCase *begin;
	struct MemoryCase *end;
	struct MemoryCase *firstHole;
	struct MemoryCase *lastHole;
	struct MemoryCase *firstProcess;
	struct MemoryCase *lastProcess;
};

struct MasterMemory{
	struct Memory *memory;
	struct MasterProcessManager *masterProcessManager;
};

typedef struct MemoryCase MemoryCase;
typedef struct Hole Hole;
typedef struct Process Process;
typedef struct EachProcessManager ProcessManager;
typedef struct MasterProcessManager MasterProcessManager;
typedef struct Memory Memory;
typedef struct MasterMemory MasterMemory;

/*--------------------FUNCTIONS HEADERS-----------------------*/

Memory * initMemory(void);
MemoryCase *findHoleThatFits(numberOfSpaces size, Memory *memory);
MemoryCase *addProcess(numberOfSpaces size, MasterMemory *masterMemory);
MemoryCase *allocProcessCase(numberOfSpaces size, MemoryCase *holeCaseThatFits, Memory* memory);
MemoryCase *overwriteHole(MemoryCase *holeCaseThatFits, Memory *memory);

MemoryCase * findNextHoleCase(MemoryCase *processCase);
MemoryCase * findPrevHoleCase(MemoryCase *processCase);

MemoryCase *nullMemoryCase(void);
MemoryCase *makeInitialMemoryCase(void);
MemoryCase *newMemoryCase(void);
MemoryCase *createAProcessCase(space begin, numberOfSpaces toUse, MemoryCase *prevProcessCase,
								  MemoryCase *nextProcessCase);
MemoryCase *removeProcessOfProcessList(MemoryCase *processCase, Memory *memory);
MemoryCase *endProcess(MemoryCase *processCase, Memory *memory);
MemoryCase *mergeHoleCases(MemoryCase *holeCaseA, MemoryCase *holeCaseB, Memory *memory);
MemoryCase *destructWithoutMerge(MemoryCase *processCase, MemoryCase *prevHoleCase, 
				  MemoryCase *nextHoleCase, Memory *memory);
Process *initProcess(space begin, numberOfSpaces inUse, MemoryCase *prevProcessCase,
							MemoryCase *nextProcessCase);
Process *newProcess(void);
Hole *makeHole(space begin, numberOfSpaces available, MemoryCase *prevHole, MemoryCase *nextHole);
Hole *makeInitialHole(void);
Hole *nullHole(void);
Hole *newHole(void);
processID newProcessID(void);
destructType selectDestructType(MemoryCase *processCase, 
				MemoryCase *prevHoleCase, 
				MemoryCase *nextHoleCase);
void printMemory(MemoryCase *firstCase);
void printProcessList(ProcessManager *firstProcessManager);
MasterMemory * initMasterMemory(void);
MasterProcessManager * initMasterProcessManager(void);
ProcessManager * nullProcessManager(void);
ProcessManager * createProcessManager(MemoryCase *head, MemoryCase *finish,
					  ProcessManager *nextProcessManager,
					  ProcessManager *prevProcessManager);
ProcessManager * mountProcessManager(MemoryCase *head, MemoryCase *finish);
numberOfSpaces getProcessManagerSize(ProcessManager *processManager);
ProcessManager * allocProcessManager(ProcessManager *toAlloc, MasterProcessManager * masterProcessManager);
MemoryCase * removeMasterProcessManager(ProcessManager *toRemove, MasterProcessManager *masterProcessManager);

/*---------------------------MAIN-----------------------------*/

int main(void){

	MasterMemory *masterMemory;
	masterMemory = initMasterMemory();

	addProcess(10, masterMemory);
	addProcess(50, masterMemory);
	addProcess(10, masterMemory);
	addProcess(70, masterMemory);
	printMemory(masterMemory->memory->begin);
	printf("\n------------------------------------------------------\n");
	printProcessList(masterMemory->masterProcessManager->firstProcessManager);

	return 0;
}

/*---------------------------MENU-----------------------------*/

/*menuOption playMenu(void){
	
}

void showOptions(){

}*/

menuOption getMenuOption(menuOption firstValue, menuOption lastValue){
	menuOption selected;
		
	do{
		scanf("%d", &selected);
	}while (selected < firstValue || selected > lastValue);

	return selected;
}

/*----------------------PRINT FUNCTIONS-----------------------*/

void printMemory(MemoryCase *firstCase){
	if(firstCase){
		if(firstCase->type == hole){
			printf("\n----> HOLE\n");
			printf("Size: %lu\n", ((Hole *)(firstCase->holeOrProcess))->available);
			printf("Begin: %lu\n", ((Hole *)(firstCase->holeOrProcess))->begin);
		}
		else{
			printf("\n----> PROCESS\n");
			printf("ID: %lu\n", ((Process *)(firstCase->holeOrProcess))->ID);
			printf("Size: %lu\n", ((Process *)(firstCase->holeOrProcess))->inUse);
			printf("Begin: %lu\n", ((Process *)(firstCase->holeOrProcess))->begin);
		}
		printMemory(firstCase->next);
	}
}

void printProcessList(ProcessManager *firstProcessManager){
	if(firstProcessManager){	
		printf("\n----> PROCESS\n");
		printf("ID: %lu\n", firstProcessManager->ID);
		printf("Size: %lu\n", firstProcessManager->size);
		printf("Begin: %lu\n", ((Process *)(firstProcessManager->head->holeOrProcess))->begin);
		printProcessList(firstProcessManager->nextProcessManager);
	}
}


/*------------------------MASTER PROCESS MANAGER FUNCTIONS-----------------------*/

MasterProcessManager * initMasterProcessManager(void){
	MasterProcessManager * newMasterProcessManager;
	newMasterProcessManager = (MasterProcessManager *) malloc (sizeof(MasterProcessManager));
	newMasterProcessManager->processCounter = 0;
	newMasterProcessManager->inUse = 0;
	newMasterProcessManager->firstProcessManager = nullProcessManager();
	newMasterProcessManager->lastProcessManager = nullProcessManager();

	return newMasterProcessManager;
}

ProcessManager * allocProcessManager(ProcessManager *toAlloc, MasterProcessManager * masterProcessManager){
	if (masterProcessManager->processCounter == 0){
		masterProcessManager->firstProcessManager = toAlloc;
		masterProcessManager->lastProcessManager = toAlloc;
		toAlloc->nextProcessManager = nullProcessManager();
		toAlloc->prevProcessManager = nullProcessManager();
	}
	else{
		toAlloc->nextProcessManager = nullProcessManager();
		toAlloc->prevProcessManager = masterProcessManager->lastProcessManager;
		toAlloc->prevProcessManager->nextProcessManager = toAlloc;
		masterProcessManager->lastProcessManager = toAlloc; 
	}
	masterProcessManager->inUse+= getProcessManagerSize(toAlloc);
	(masterProcessManager->processCounter)++;

	return toAlloc;
}

MemoryCase * removeMasterProcessManager(ProcessManager *toRemove, MasterProcessManager *masterProcessManager){
	MemoryCase *firstMemoryCaseAtList;	
		
	if(masterProcessManager->firstProcessManager == toRemove)
		masterProcessManager->firstProcessManager = toRemove->nextProcessManager;
	else
		toRemove->prevProcessManager->nextProcessManager = toRemove->nextProcessManager;
	
	if(masterProcessManager->lastProcessManager == toRemove)
		masterProcessManager->lastProcessManager = toRemove->prevProcessManager;
	else
		toRemove->nextProcessManager->prevProcessManager = toRemove->prevProcessManager;

	firstMemoryCaseAtList = toRemove->head;
	masterProcessManager->inUse-= getProcessManagerSize(toRemove);
	(masterProcessManager->processCounter)--;
	free(toRemove);

	return firstMemoryCaseAtList;
}

ProcessManager * syncMasterProcessManager(MemoryCase *head, MemoryCase *finish, MasterProcessManager *masterProcessManager){
	ProcessManager *newProcessManager;
	newProcessManager = mountProcessManager(head, finish);
	return allocProcessManager(newProcessManager, masterProcessManager);
}
/*------------------------EACH PROCESS MANAGER FUNCTIONS-------------------------*/

ProcessManager * nullProcessManager(void){
	return NULL;
}

numberOfSpaces getProcessManagerSize(ProcessManager *processManager){
	MemoryCase *runner = processManager->head;
	numberOfSpaces size = 0;

	while(1){
		size += ((Process *)(runner->holeOrProcess))->inUse;
		if(runner == processManager->finish) break;
		runner = ((Process *)(runner->holeOrProcess))->nextProcess;
	}
	return size;
}

ProcessManager * createProcessManager(MemoryCase *head, MemoryCase *finish,
					  ProcessManager *nextProcessManager,
					  ProcessManager *prevProcessManager){
	ProcessManager * newProcessManager;
	newProcessManager = (ProcessManager *) malloc (sizeof(ProcessManager));	
	newProcessManager->head = head;
	newProcessManager->finish = finish;
	newProcessManager->nextProcessManager = nextProcessManager;
	newProcessManager->prevProcessManager = prevProcessManager;
	newProcessManager->ID = ((Process *)(head->holeOrProcess))->ID;
	newProcessManager->size = getProcessManagerSize(newProcessManager);

	return newProcessManager;
}

ProcessManager * mountProcessManager(MemoryCase *head, MemoryCase *finish){
	ProcessManager *mountedProcessManager;
	mountedProcessManager = createProcessManager(head, finish, nullProcessManager(), nullProcessManager());
	return mountedProcessManager;
}

/*------------------------MASTER MEMORY FUNCTIONS------------------------*/

MasterMemory * initMasterMemory(void){
	MasterMemory *newMasterMemory;

	newMasterMemory = (MasterMemory *) malloc (sizeof(MasterMemory));
	newMasterMemory->memory = initMemory();
	newMasterMemory->masterProcessManager = initMasterProcessManager();

	return newMasterMemory;
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

MemoryCase * addProcess(numberOfSpaces size, MasterMemory* masterMemory){	
	numberOfSpaces remaning = size;
	MemoryCase *runner = masterMemory->memory->firstHole;
	MemoryCase *processListBegin = runner;
	MemoryCase *processListEnd;
	Hole *runnerHole;	
	MemoryCase *holeCaseThatFits;	

	holeCaseThatFits = findHoleThatFits(size, masterMemory->memory);
	
	if(!holeCaseThatFits)		
		return nullMemoryCase();

	while(1){
		runnerHole = (Hole *) runner->holeOrProcess;
		if(runnerHole->available < remaning){
			remaning-=runnerHole->available;
			if(runner == masterMemory->memory->firstHole)
				masterMemory->memory->firstHole = runnerHole->nextHole;
			overwriteHole(runner, masterMemory->memory);		
		}
		else if(runnerHole->available == remaning){
			if(runner == masterMemory->memory->firstHole)
				masterMemory->memory->firstHole = runnerHole->nextHole;
			if(runner == masterMemory->memory->lastHole)
				masterMemory->memory->lastHole = runnerHole->prevHole;

			overwriteHole(runner, masterMemory->memory);
			break;
		}
		else{	
			processListEnd = allocProcessCase(size, runner, masterMemory->memory);
			if (holeCaseThatFits == masterMemory->memory->firstHole)
				processListBegin = processListEnd;
			if(runner == masterMemory->memory->begin)
				masterMemory->memory->begin = processListBegin;
			break;
		}
		runner = runnerHole->nextHole;
	}

	syncMasterProcessManager(processListBegin, processListEnd, masterMemory->masterProcessManager);
	
	return processListBegin;
}

MemoryCase *findHoleThatFits(numberOfSpaces size, Memory *memory){	
	numberOfSpaces remaning;
	Hole * currentHole;
	MemoryCase *currentHoleCase = memory->firstHole;
	currentHole = (Hole *) memory->firstHole->holeOrProcess;
	
	remaning = size;		
	while(currentHoleCase && currentHole->available < remaning){
		remaning -= currentHole->available;
		if(currentHole->nextHole){
			currentHoleCase = currentHole->nextHole;
			currentHole = (Hole *) currentHoleCase->holeOrProcess;
		}
		else return nullMemoryCase();
	}

	return currentHoleCase;
}

MemoryCase *allocProcessCase(numberOfSpaces size, MemoryCase *holeCaseThatFits, Memory *memory){
	MemoryCase *processCase;
	Hole *theHole;

	theHole =  (Hole *) holeCaseThatFits->holeOrProcess;
	processCase = createAProcessCase(theHole->begin, size, memory->lastProcess, nullMemoryCase());
	
	if(memory->lastProcess)
		((Process *)(memory->lastProcess->holeOrProcess))->nextProcess = processCase;	
	memory->lastProcess = processCase;
	if(!memory->firstProcess)
		memory->firstProcess = processCase;
	
	(theHole->available) -= size;
	(theHole->begin) += size;

	processCase->next = holeCaseThatFits;
	processCase->prev = holeCaseThatFits->prev;
	if(processCase->prev)
		processCase->prev->next = processCase;	
	holeCaseThatFits->prev = processCase;
	
	return processCase; 
}

MemoryCase * overwriteHole(MemoryCase *holeCaseThatFits, Memory *memory){
	Hole *theHole;
	theHole =  (Hole *) holeCaseThatFits->holeOrProcess;
	
	if(theHole->prevHole)
		((Hole *)(theHole->prevHole->holeOrProcess))->nextHole = theHole->nextHole;
	if(theHole->nextHole)	
		((Hole *)(theHole->nextHole->holeOrProcess))->prevHole = theHole->prevHole;	

	holeCaseThatFits->type = process;
	holeCaseThatFits->holeOrProcess = (void *) initProcess(theHole->begin, theHole->available,
								memory->lastProcess, nullMemoryCase());

	if(memory->lastProcess)
		((Process *)(memory->lastProcess->holeOrProcess))->nextProcess = holeCaseThatFits;	
	memory->lastProcess = holeCaseThatFits;

	if(!memory->firstProcess)
		memory->firstProcess = holeCaseThatFits;

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
	MemoryCase *holeCase;

	holeCase = removeProcessOfProcessList(processCase, memory);	
	holeCase->type = hole;

	aHole = makeHole( ((Process *)(processCase->holeOrProcess))->begin, 
			    ((Process *)(processCase->holeOrProcess))->inUse,
			    prevHoleCase, nextHoleCase);
	holeCase->holeOrProcess = (void *) aHole;

	if (nextHoleCase)
		((Hole *)(nextHoleCase->holeOrProcess))->prevHole = holeCase;	
	else memory->lastHole = holeCase;
	if (prevHoleCase)
		((Hole *)(prevHoleCase->holeOrProcess))->nextHole = holeCase;
	else memory->firstHole = holeCase;

	return holeCase;
}

MemoryCase * removeProcessOfProcessList(MemoryCase *processCase, Memory *memory){

	if(((Process *)(processCase->holeOrProcess))->prevProcess)
		((Process*)(((Process *)(processCase->holeOrProcess))->prevProcess->holeOrProcess))->nextProcess = ((Process *)(processCase->holeOrProcess))->nextProcess;
	else memory->firstProcess = ((Process *)(processCase->holeOrProcess))->nextProcess;

	if(((Process *)(processCase->holeOrProcess))->nextProcess)
		((Process*)(((Process *)(processCase->holeOrProcess))->nextProcess->holeOrProcess))->prevProcess = ((Process *)(processCase->holeOrProcess))->prevProcess;
	else memory->lastProcess = ((Process *)(processCase->holeOrProcess))->prevProcess;

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

	if (memory->firstHole == holeCaseA)
		memory->firstHole = holeCaseB;	

	free(holeCaseA);
	return holeCaseB;
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

MemoryCase * createAProcessCase(space begin, numberOfSpaces toUse, MemoryCase *prevProcessCase,
								   MemoryCase *nextProcessCase){
	MemoryCase *newProcessCase;
	newProcessCase = newMemoryCase();
	newProcessCase->type = process;
	newProcessCase->next = nullMemoryCase();
	newProcessCase->prev = nullMemoryCase();
	newProcessCase->holeOrProcess = (void *) initProcess(begin, toUse, 
							     prevProcessCase,
							     nextProcessCase);

	return newProcessCase;
}

MemoryCase *nullMemoryCase(void){
	return NULL;
}

MemoryCase *newMemoryCase(void){
	return (MemoryCase *) malloc (sizeof(MemoryCase));
}

/*---------------------PROCESS FUNCTIONS----------------------*/

Process *initProcess(space begin, numberOfSpaces inUse, MemoryCase *prevProcessCase,
							MemoryCase *nextProcessCase){
	Process * aProcess;
	aProcess = newProcess();
	aProcess->begin = begin;
	aProcess->inUse = inUse;
	aProcess->ID = newProcessID();
	aProcess->prevProcess = prevProcessCase;
	aProcess->nextProcess = nextProcessCase;

	return aProcess;
}

Process *newProcess(void){
	return (Process *) malloc (sizeof(Process));
}

processID newProcessID(void){
	static processID numberOfProcess = 0;
	return numberOfProcess++;
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



















