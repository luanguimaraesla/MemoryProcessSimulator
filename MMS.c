/* Memory Manager Simulator */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>

#define DESTRUCT_WITHOUT_MERGE  0
#define DESTRUCT_MERGE_NEXT     1
#define DESTRUCT_MERGE_PREV     2
#define DESTRUCT_MERGE_BOTH     3
#define MAX_PROCESS_SIZE 50
#define MAX_PRIORITY_INDEX 5
#define MAX_PROCESS_GENERATE_SLEEP 70
#define SCREEN_UPDATE_FREQUENCY 60
#define MAX_PROCESS_TIME 7

/*------------------------TYPEDEFS-------------------------*/

typedef unsigned int priority;
typedef unsigned long space;
typedef unsigned long numberOfSpaces;
typedef unsigned long processID;
typedef unsigned long numberOfProcesses;
typedef int destructType;
typedef int insertionMode;

/*--------------------------ENUMS--------------------------*/

enum bool{
	false, true
};

enum memoryCaseType{
	hole, process
};

typedef enum memoryCaseType memoryCaseType;
typedef enum bool bool;

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
	pthread_t execute;
	processID id;
	priority index;
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

struct ExecProcessArg{
	struct Memory *memory;
	struct MemoryCase *selfCase;
};

struct RandomCreateProcessesArg{
	struct MemoryCase * (*addProcessFunction)(numberOfSpaces, priority, struct Memory *);
	unsigned int numberOfProcesses;
	numberOfSpaces maxProcessSize;
	priority maxPriorityIndex;
	unsigned int maxProcessGenerateSleep;
	struct Memory *memory;
};

struct FrameUpdateArg{
	bool * frame_update;
	struct Memory *memory;
};

typedef struct MemoryCase MemoryCase;
typedef struct Process Process;
typedef struct Hole Hole;
typedef struct Memory Memory;
typedef struct ExecProcessArg execution_arg;
typedef struct RandomCreateProcessesArg rcp_arg;
typedef struct FrameUpdateArg fu_arg;

/*--------------------GLOBAL VARIABLES---------------------*/

pthread_mutex_t mutex_listModify;
pthread_cond_t cond_listModify;

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
MemoryCase * createProcessCase(processID id, priority index, space begin, numberOfSpaces size,
							   MemoryCase *nextProcessCase, MemoryCase *prevProcessCase,
							   MemoryCase *next, MemoryCase *prev);
Hole * createHole(MemoryCase *nextHoleCase, MemoryCase *prevHoleCase);
Process * createProcess(processID id, priority index, MemoryCase *nextProcessCase, MemoryCase *prevProcessCase);
MemoryCase * createInitialHoleCase(numberOfSpaces size);

/*3. Null functions*/

MemoryCase * nullMemoryCase(void);

/*4. Auxiliar functions*/

processID getNewProcessID(void);

/*5. Add process functions*/

MemoryCase *addProcessFirstFit(numberOfSpaces size, priority index, Memory *memory);
MemoryCase *reallocAndInsert_best(numberOfSpaces size, priority index, MemoryCase *insertBegin, Memory *memory);
void removeHoleCase(MemoryCase *toRemove, Memory *memory);
MemoryCase *overwriteHoleCase(priority index, MemoryCase *holeToOverwrite, Memory *memory);
MemoryCase *findNextPriorityListProcessCase(priority index, MemoryCase *firstProcessCase);
MemoryCase *divideAndInsert(numberOfSpaces size, priority index, MemoryCase * holeCaseToDivide, Memory *memory);
MemoryCase *findTheBestFitHoleCase(numberOfSpaces size, Memory *memory);
MemoryCase *reallocAndInsert_worst(numberOfSpaces size, priority index, MemoryCase *insertBegin, Memory *memory);
MemoryCase *findTheWorstFitHoleCase(numberOfSpaces size, Memory *memory);
MemoryCase * addProcessWorstFit(numberOfSpaces size, priority index, Memory *memory);
MemoryCase * addProcessBestFit(numberOfSpaces size, priority index, Memory *memory);
MemoryCase *findPrevPriorityListProcessCase(priority index, MemoryCase *firstProcessCase);

/*6. Print functions*/

void printMemory(Memory *memory);
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
numberOfSpaces getSize(void);
void ask_rcp_args(rcp_arg *args);

/*9. thread functions*/
void * executeProcess(void *vargp);
void * randomCreateProcesses(void *vargp);
void * plotMemoryStatus(void *vargp);


/*-------------------------MAIN----------------------------*/

int main(void){

	Memory *memory;
	rcp_arg args;
	pthread_t processesCreation, frameUpdate;
	bool frame_update = true;
	fu_arg frame_update_args;

	memory = createMemory(getSize());
	args.memory = memory;
	ask_rcp_args(&args);

	frame_update_args.memory = memory;
	frame_update_args.frame_update = &frame_update;
	
	pthread_mutex_init(&mutex_listModify, NULL);
	pthread_create(&processesCreation, NULL, randomCreateProcesses, &args);
	pthread_create(&frameUpdate, NULL, plotMemoryStatus, &frame_update_args);


	pthread_join(processesCreation, NULL);

	while(memory->firstProcessCase);
	sleep(1);
	frame_update = false;
	pthread_join(frameUpdate, NULL);
	pthread_mutex_destroy(&mutex_listModify);
	
	pthread_exit((void *)NULL);
	
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

numberOfSpaces getSize(void){
	numberOfSpaces size;
	printMemorySizeMenu();
	scanf("%lu", &size);
	return size;
}

void ask_rcp_args(rcp_arg *args){

	switch(getInsertionMode()){
		case 1:
			args->addProcessFunction = addProcessFirstFit;
			break;
		case 2:
			args->addProcessFunction = addProcessWorstFit;
			break;
		case 3:
			args->addProcessFunction = addProcessBestFit;
			break;
		default:
			printf("\nERROR!\n");
			exit(1); 
	}
	
	printf("\nHow many processes do you want to create? ");
	scanf("%u", &(args->numberOfProcesses));

	printf("Enter the maximum each process size: ");
	scanf("%lu", &(args->maxProcessSize));

	printf("Enter the maximum each process priority value: ");
	scanf("%u", &(args->maxPriorityIndex));

	printf("Enter the longest interval between creation processes: ");
	scanf("%u", &(args->maxProcessGenerateSleep));

}

/*--------------------PRINT FUNCTIONS----------------------*/

void printMemory(Memory *memory){
	MemoryCase *firstPrintedCase = memory->begin;
	MemoryCase *firstCase = memory->begin;
	
	/*clean screen*/
	printf("\e[H\e[2J");
	printf("\n--------------MEMORY STATUS-------------");
	printf("\nAvailable: %lu\nUsing: %lu\n", memory->available, memory->inUse);

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
		printf("Priority: %d\n", ((Process *)(firstProcessCase->holeOrProcess))->index);
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

MemoryCase *addProcessWorstFit(numberOfSpaces size, priority index, Memory *memory){
	MemoryCase *worstFitHoleCase = findTheWorstFitHoleCase(size, memory);
	MemoryCase *response;
	execution_arg *exec_arg;

	if(memory->available < size || !worstFitHoleCase)
		return nullMemoryCase();
	if(size >= worstFitHoleCase->size)
		response = reallocAndInsert_worst(size, index, worstFitHoleCase, memory);
	else
		response = divideAndInsert(size, index, worstFitHoleCase, memory);

	exec_arg = (execution_arg *) malloc (sizeof(execution_arg));
	exec_arg->selfCase = response;
	exec_arg->memory = memory;
	pthread_create(&(((Process *)(response->holeOrProcess))->execute), NULL, executeProcess, (void *)exec_arg );

	return response;
}

MemoryCase *reallocAndInsert_worst(numberOfSpaces size, priority index, MemoryCase *insertBegin, Memory *memory){
	MemoryCase *currentPrevHoleCase = ((Hole*)(insertBegin->holeOrProcess))->prevHoleCase;
	MemoryCase *currentNextHoleCase = ((Hole*)(insertBegin->holeOrProcess))->nextHoleCase;
	MemoryCase *runner;
	numberOfSpaces currentSizeAux = 0;
	numberOfSpaces currentPrevSize = 0;
	numberOfSpaces currentNextSize = 0;
	
	while(insertBegin->size + currentPrevSize + currentNextSize < size){
		if(abs((size - (currentPrevSize + currentNextSize)) - currentPrevHoleCase->size) >
		   abs((size - (currentPrevSize + currentNextSize)) - currentNextHoleCase->size)){
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
			runner->begin = (currentNextSize - currentSizeAux) + runner->begin >= memory->available + memory->inUse ?
					 runner->begin + (currentNextSize - currentSizeAux) - memory->available - memory->inUse :
					 runner->begin + (currentNextSize - currentSizeAux);
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
				runner->begin += (currentNextSize - currentSizeAux) + runner->begin >= memory->available + memory->inUse ?
					 	 runner->begin + (currentNextSize - currentSizeAux) - memory->available - memory->inUse :
						 runner->begin + (currentNextSize - currentSizeAux);
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
			runner->begin = (signed long)(runner->begin) - (signed long)(currentPrevSize - currentSizeAux) < 0 ?
					memory->available + memory->inUse - abs((signed long)(runner->begin) - (signed long)(currentPrevSize - currentSizeAux)) :
					runner->begin - (currentPrevSize - currentSizeAux);		
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
						insertBegin->prev->size + insertBegin->prev->begin - memory->inUse - memory->available :
						insertBegin->prev->size + insertBegin->prev->begin;
	
	memory->available -= size;
	memory->inUse+=size;

	return overwriteHoleCase(index, insertBegin, memory);
}

/*FIRST FIT INSERTION*/

MemoryCase *reallocAndInsert_best(numberOfSpaces size, priority index, MemoryCase *insertBegin, Memory *memory){
	MemoryCase *currentPrevHoleCase = ((Hole*)(insertBegin->holeOrProcess))->prevHoleCase;
	MemoryCase *currentNextHoleCase = ((Hole*)(insertBegin->holeOrProcess))->nextHoleCase;
	MemoryCase *runner;
	numberOfSpaces currentSizeAux = 0;
	numberOfSpaces currentPrevSize = 0;
	numberOfSpaces currentNextSize = 0;
	
	while(insertBegin->size + currentPrevSize + currentNextSize < size){
		if(abs((size - (currentPrevSize + currentNextSize)) - currentPrevHoleCase->size) <
		   abs((size - (currentPrevSize + currentNextSize)) - currentNextHoleCase->size)){
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
			runner->begin = (currentNextSize - currentSizeAux) + runner->begin >= memory->available + memory->inUse ?
					 	 runner->begin + (currentNextSize - currentSizeAux) - memory->available - memory->inUse :
						 runner->begin + (currentNextSize - currentSizeAux);
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
				runner->begin = (currentNextSize - currentSizeAux) + runner->begin >= memory->available + memory->inUse ?
					 	 runner->begin + (currentNextSize - currentSizeAux) - memory->available - memory->inUse :
						 runner->begin + (currentNextSize - currentSizeAux);
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
			runner->begin = (signed long)(runner->begin) - (signed long)(currentPrevSize - currentSizeAux) < 0 ?
					memory->available + memory->inUse - abs((signed long)(runner->begin) - (signed long)(currentPrevSize - currentSizeAux)) :
					runner->begin - (currentPrevSize - currentSizeAux);
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
	insertBegin->begin = (insertBegin->prev->size + insertBegin->prev->begin) >= memory->inUse + memory->available ?
						insertBegin->prev->size + insertBegin->prev->begin - memory->inUse - memory->available :
						insertBegin->prev->size + insertBegin->prev->begin;
	
	memory->available -= size;
	memory->inUse+=size;

	return overwriteHoleCase(index, insertBegin, memory);
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

MemoryCase *overwriteHoleCase(priority index, MemoryCase *holeToOverwrite, Memory *memory){
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

	nextProcessCase = findNextPriorityListProcessCase(index, memory->firstProcessCase);
	prevProcessCase = findPrevPriorityListProcessCase(index, memory->firstProcessCase);	
	holeToOverwrite->holeOrProcess = createProcess(getNewProcessID(), index, nextProcessCase, prevProcessCase);
	
	if(nextProcessCase == memory->firstProcessCase)
		memory->firstProcessCase = holeToOverwrite;
	if(nextProcessCase)
		((Process*)(nextProcessCase->holeOrProcess))->prevProcessCase = holeToOverwrite;
	if(prevProcessCase)
		((Process*)(prevProcessCase->holeOrProcess))->nextProcessCase = holeToOverwrite;

	holeToOverwrite->type = process;

	return holeToOverwrite;
}

MemoryCase *findNextPriorityListProcessCase(priority index, MemoryCase *firstProcessCase){
	if(!firstProcessCase) return nullMemoryCase();
	else if (((Process *)(firstProcessCase->holeOrProcess))->index >= index) return firstProcessCase;
	else return findNextPriorityListProcessCase(index, ((Process *)(firstProcessCase->holeOrProcess))->nextProcessCase);
}

MemoryCase *findPrevPriorityListProcessCase(priority index, MemoryCase *firstProcessCase){
	MemoryCase *prevProcessCase = nullMemoryCase();
	while(firstProcessCase && index > ((Process *)(firstProcessCase->holeOrProcess))->index){
			prevProcessCase = firstProcessCase;
			firstProcessCase = ((Process *)(firstProcessCase->holeOrProcess))->nextProcessCase;
	}
	return prevProcessCase;
}

MemoryCase *addProcessFirstFit(numberOfSpaces size, priority index, Memory *memory){
	MemoryCase *firstHoleCase = memory->firstHoleCase;
	MemoryCase *response;
	execution_arg *exec_arg;

	if(memory->available < size || !firstHoleCase)
		return nullMemoryCase();

	if(size >= firstHoleCase->size)
		response = reallocAndInsert_best(size, index, firstHoleCase, memory);
	else
		response = divideAndInsert(size, index, firstHoleCase, memory);

	exec_arg = (execution_arg *) malloc (sizeof(execution_arg));
	exec_arg->selfCase = response;
	exec_arg->memory = memory;
	pthread_create(&(((Process *)(response->holeOrProcess))->execute), NULL, executeProcess, (void *)exec_arg );

	return response;
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

MemoryCase *addProcessBestFit(numberOfSpaces size, priority index, Memory *memory){
	MemoryCase *bestFitHoleCase = findTheBestFitHoleCase(size, memory);
	MemoryCase *response;
	execution_arg *exec_arg;

	if(memory->available < size || !bestFitHoleCase)
		return nullMemoryCase();
	if(size >= bestFitHoleCase->size)
		response = reallocAndInsert_best(size, index, bestFitHoleCase, memory);
	else
		response = divideAndInsert(size, index, bestFitHoleCase, memory);

	exec_arg = (execution_arg *) malloc (sizeof(execution_arg));
	exec_arg->selfCase = response;
	exec_arg->memory = memory;
	pthread_create(&(((Process *)(response->holeOrProcess))->execute), NULL, executeProcess, (void *)exec_arg );

	return response;
}

MemoryCase *divideAndInsert(numberOfSpaces size, priority index, MemoryCase * holeCaseToDivide, Memory *memory){
	MemoryCase *newProcessCase;
	MemoryCase *nextProcessCase;
	MemoryCase *prevProcessCase;

	nextProcessCase = findNextPriorityListProcessCase(index, memory->firstProcessCase);
	prevProcessCase = findPrevPriorityListProcessCase(index, memory->firstProcessCase);
	newProcessCase = createProcessCase(getNewProcessID(), index, holeCaseToDivide->begin, size,
					   nextProcessCase, prevProcessCase, holeCaseToDivide, holeCaseToDivide->prev);

	holeCaseToDivide->size -= size;
	holeCaseToDivide->begin = (size + holeCaseToDivide->begin) > memory->inUse + memory->available?
				   size + holeCaseToDivide->begin - memory->inUse - memory->available :
				   size + holeCaseToDivide->begin;
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

	memory->available -= size;
	memory->inUse+=size;

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
	else{
		memory->available += processCase->size;
		memory->inUse-= processCase->size;
	}
	
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

MemoryCase * createProcessCase(processID id, priority index, space begin, numberOfSpaces size,
							   MemoryCase *nextProcessCase, MemoryCase *prevProcessCase,
							   MemoryCase *next, MemoryCase *prev){
	MemoryCase *newProcessCase;
	newProcessCase = createMemoryCase(process, begin, size, next, prev);
	newProcessCase->holeOrProcess = (void *) createProcess(id,index, nextProcessCase, prevProcessCase);

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

Process * createProcess(processID id, priority index, 
						MemoryCase *nextProcessCase, 
						MemoryCase *prevProcessCase){
	Process *newProcess;

	newProcess = allocProcess();
	newProcess->id = id;
	newProcess->index = index;
	newProcess->nextProcessCase = nextProcessCase;
	newProcess->prevProcessCase = prevProcessCase;

	return newProcess;
}

/*----------------------AUXILIAR---------------------------*/

processID getNewProcessID(void){
	static processID id = -1;
	return ++id;
}

/*--------------------THREADS FUNCTIONS-----------------------*/

void * executeProcess(void *vargp){
	execution_arg *exec_arg = (execution_arg *) vargp;	
	time_t t;
	srand((unsigned) time(&t));
	sleep(1 + rand() % (MAX_PROCESS_TIME - 1));

	pthread_mutex_lock(&mutex_listModify);
	endProcess(exec_arg->selfCase, exec_arg->memory);
	pthread_mutex_unlock(&mutex_listModify);

	pthread_exit((void *)NULL);
}

void * randomCreateProcesses(void *vargp){
	rcp_arg *args = (rcp_arg *) vargp;
	time_t t;
	MemoryCase *(*addProcessFunction)() = args->addProcessFunction;
	srand((unsigned) time(&t));
	for(;args->numberOfProcesses;(args->numberOfProcesses)--){
		pthread_mutex_lock(&mutex_listModify);
		(*addProcessFunction)(1 + rand() % (args->maxProcessSize - 1), rand() % args->maxPriorityIndex, args->memory);
		pthread_mutex_unlock(&mutex_listModify);
		sleep(rand() % args->maxProcessGenerateSleep);
	}

	pthread_exit((void *)NULL);
}

void * plotMemoryStatus(void *vargp){
	fu_arg *args = (fu_arg *) vargp;
	
	while(*(args->frame_update)){
		pthread_mutex_lock(&mutex_listModify);
		printf("\e[H\e[2J");
		printMemory(args->memory);
		pthread_mutex_unlock(&mutex_listModify);
		sleep(1);
	}

	pthread_exit((void *)NULL);	
}









