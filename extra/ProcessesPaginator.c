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
MemoryCase *allocProcessCase(processID id, numberOfSpaces size, MemoryCase *holeCaseThatFits, Memory* memory);
MemoryCase *overwriteHole(processID id, MemoryCase *holeCaseThatFits, Memory *memory);

MemoryCase * findNextHoleCase(MemoryCase *processCase);
MemoryCase * findPrevHoleCase(MemoryCase *processCase);

MemoryCase *nullMemoryCase(void);
MemoryCase *makeInitialMemoryCase(void);
MemoryCase *newMemoryCase(void);
MemoryCase *createAProcessCase(processID id, space begin, numberOfSpaces toUse, MemoryCase *prevProcessCase,
								  MemoryCase *nextProcessCase);
MemoryCase *removeProcessOfProcessList(MemoryCase *processCase, Memory *memory);
MemoryCase *endProcess(MemoryCase *processCase, MasterMemory *masterMemory);
MemoryCase *mergeHoleCases(MemoryCase *holeCaseA, MemoryCase *holeCaseB, Memory *memory);
MemoryCase *destructWithoutMerge(MemoryCase *processCase, MemoryCase *prevHoleCase, 
				  MemoryCase *nextHoleCase, Memory *memory);
Process *initProcess(processID id, space begin, numberOfSpaces inUse, MemoryCase *prevProcessCase,
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
MemoryCase * removeProcessManager(ProcessManager *toRemove, MasterProcessManager *masterProcessManager);
MemoryCase *endProcessByID(processID ID, MasterMemory *masterMemory);
ProcessManager *searchProcessManagerByID(processID ID, ProcessManager *firstProcessManager);

/*---------------------------MAIN-----------------------------*/

int main(void){

	MasterMemory *masterMemory;
	masterMemory = initMasterMemory();

	addProcess(10, masterMemory);
	printMemory(masterMemory->memory->begin);
	printf("\n------------------------------------------------------\n");

	addProcess(50, masterMemory);
	printMemory(masterMemory->memory->begin);
	printf("\n------------------------------------------------------\n");

	addProcess(10, masterMemory);
	printMemory(masterMemory->memory->begin);
	printf("\n------------------------------------------------------\n");

	addProcess(70, masterMemory);
	printMemory(masterMemory->memory->begin);
	printf("\n------------------------------------------------------\n");
	
	addProcess(30, masterMemory);
	printMemory(masterMemory->memory->begin);
	printf("\n------------------------------------------------------\n");

	endProcessByID(0, masterMemory);
	printMemory(masterMemory->memory->begin);
	printf("\n------------------------------------------------------\n");

	endProcessByID(3, masterMemory);
	printMemory(masterMemory->memory->begin);
	printf("\n------------------------------------------------------\n");

	addProcess(40, masterMemory);
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



/* Função que percorre recursivamente a lista principal da memória imprimindo todos os elementos
   independentemente de ser processo ou buraco */

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



/* Função que percorre recursivamente todos os cabeçalhos dos processos, imprimindo a posição
   da primeira ocorrência na memória e quantas posições ele ocupa no total, além disso, fornece
   o identificador de cada processo */

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


/* Inicializa, alocando dinamicamente a estrutura que reúne todos os cabeçalhos de processo */

MasterProcessManager * initMasterProcessManager(void){
	MasterProcessManager * newMasterProcessManager;
	newMasterProcessManager = (MasterProcessManager *) malloc (sizeof(MasterProcessManager));
	newMasterProcessManager->processCounter = 0;
	newMasterProcessManager->inUse = 0;
	newMasterProcessManager->firstProcessManager = nullProcessManager();
	newMasterProcessManager->lastProcessManager = nullProcessManager();

	return newMasterProcessManager;
}

/* Aloca um novo cabeçalho de processo dentro da estrutura que reúne esses cabeçalhos, inserindo
   esse novo processo no final da lista duplamente encadeada */

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


/* Realiza o procedimento contrário à função anterior, remove um elemento qualquer da lista, 
   essa remoção se dá em qualquer parte da lista, para isso é necessário passar o endereço 
   do elemento que deve ser removido */

MemoryCase * removeProcessManager(ProcessManager *toRemove, MasterProcessManager *masterProcessManager){
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
	masterProcessManager->inUse-= toRemove->size;
	(masterProcessManager->processCounter)--;
	free(toRemove);

	return firstMemoryCaseAtList;
}


/* Função que realiza uma busca recursiva por uma lista de cabeçalhos de processo, recebe o
   identificador desse processo e retorna o endereço de memória ocorrência desse identificador
   entre os elementos */

ProcessManager *searchProcessManagerByID(processID ID, ProcessManager *firstProcessManager){
	if (firstProcessManager == NULL) return nullProcessManager();
	else if (firstProcessManager->ID == ID) return firstProcessManager;
	else return searchProcessManagerByID(ID, firstProcessManager->nextProcessManager);
}


/* Função utilizada para sincronizar o cabeçalho pai dos processos com a memória. Cada vez que um novo
   processo é criado, essa função é chamada para alocar os devidos apontadores que marcam o início
   e o fim desse processo, já que o mesmo pode se encontrar de forma desordenada dentro da memória */

ProcessManager * syncMasterProcessManager(MemoryCase *head, MemoryCase *finish, MasterProcessManager *masterProcessManager){
	ProcessManager *newProcessManager;
	newProcessManager = mountProcessManager(head, finish);
	return allocProcessManager(newProcessManager, masterProcessManager);
}

/*------------------------EACH PROCESS MANAGER FUNCTIONS-------------------------*/


/* Retorna um cabeçalho de processo Nulo */

ProcessManager * nullProcessManager(void){
	return NULL;
}


/* Função que percorre os nós de um determinado processo recuperando o tamanho total de todos os
   segmentos reunidos. */

numberOfSpaces getProcessManagerSize(ProcessManager *processManager){
	MemoryCase *runner = processManager->head;
	numberOfSpaces size = 0;

	while(1){
		if (((Process *)(runner->holeOrProcess))->ID == 
		    ((Process *)(processManager->head->holeOrProcess))->ID){
			size += ((Process *)(runner->holeOrProcess))->inUse;
		}
		if(runner == processManager->finish) break;
		runner = ((Process *)(runner->holeOrProcess))->nextProcess;
	}
	return size;
}


/* Função que cria o cabeçalho de um processo a partir do primeiro elemento, do último e estabelece
   os nós de outros cabeçalhos de processo imediatamente ao seu lado */

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


/* Constrói um cabeçalho de processo vazio que não possui alocação, ou seja,
   um cabeçalho solto no espaço. */

ProcessManager * mountProcessManager(MemoryCase *head, MemoryCase *finish){
	ProcessManager *mountedProcessManager;
	mountedProcessManager = createProcessManager(head, finish, nullProcessManager(), nullProcessManager());
	return mountedProcessManager;
}

/*------------------------MASTER MEMORY FUNCTIONS------------------------*/

/* Função de inicialização da estrutura principal do programa. São inicializadas
   automaticamente a lista de cabeçalhos de processo e a memória */

MasterMemory * initMasterMemory(void){
	MasterMemory *newMasterMemory;

	newMasterMemory = (MasterMemory *) malloc (sizeof(MasterMemory));
	newMasterMemory->memory = initMemory();
	newMasterMemory->masterProcessManager = initMasterProcessManager();

	return newMasterMemory;
}

/* Função que encerra um processo com um determinado identificador */

MemoryCase *endProcessByID(processID ID, MasterMemory *masterMemory){
	ProcessManager *toRemove;
	MemoryCase *processCase;
	int setEnd = 0;

	toRemove = searchProcessManagerByID(ID, masterMemory->masterProcessManager->firstProcessManager);	

	if(!toRemove)
		return nullMemoryCase();

	if(toRemove)
	
	processCase = toRemove->head;
	while(!setEnd){
		if(processCase == toRemove->finish)
			setEnd = 1;
		if (((Process *)(processCase->holeOrProcess))->ID == ID)
			endProcess(processCase, masterMemory);
		if(!setEnd)
			processCase = ((Process*)(processCase->holeOrProcess))->nextProcess;
	}

	return removeProcessManager(toRemove, masterMemory->masterProcessManager);
}

/*---------------------MEMORY FUNCTIONS-----------------------*/


/*Função de inicialização da memória. Cria um único espaço vazio de tamanho determinado pelo usuário
  durante a inicialização do programa. */

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

/* Função que adiciona um processo na memória. Deve-se fonecer o tamanho desse processo */

MemoryCase * addProcess(numberOfSpaces size, MasterMemory* masterMemory){
	processID id;	
	numberOfSpaces remaning = size;
	MemoryCase *firstHoleCase = masterMemory->memory->firstHole;
	MemoryCase *runner = masterMemory->memory->firstHole;
	MemoryCase *processListBegin = runner;
	MemoryCase *processListEnd;
	Hole *runnerHole;	
	MemoryCase *holeCaseThatFits;	

	holeCaseThatFits = findHoleThatFits(size, masterMemory->memory);
	if(!holeCaseThatFits)		
		return nullMemoryCase();
	
	id = newProcessID();

	while(1){
		runnerHole = (Hole *) runner->holeOrProcess;
		if(runnerHole->available < remaning){
			remaning-=runnerHole->available;
			if(runner == masterMemory->memory->firstHole)
				masterMemory->memory->firstHole = runnerHole->nextHole;
			overwriteHole(id, runner, masterMemory->memory);		
		}
		else if(runnerHole->available == remaning){
			if(runner == masterMemory->memory->firstHole)
				masterMemory->memory->firstHole = runnerHole->nextHole;
			if(runner == masterMemory->memory->lastHole)
				masterMemory->memory->lastHole = runnerHole->prevHole;

			overwriteHole(id, runner, masterMemory->memory);
			break;
		}
		else{	
			processListEnd = allocProcessCase(id, remaning, runner, masterMemory->memory);
			if (holeCaseThatFits == firstHoleCase){
				processListBegin = processListEnd;
			}
			if(runner == masterMemory->memory->begin)
				masterMemory->memory->begin = processListBegin;
			break;
		}
		runner = runnerHole->nextHole;
	}

	syncMasterProcessManager(processListBegin, processListEnd, masterMemory->masterProcessManager);
	
	return processListBegin;
}


/* Função que devolve a última posição necessária para encaixar completamente um processo de tamanho
   'size' dentro da memória. */

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


/* Função que aloca um processo em um buraco maior que ele, ou seja, cria um nó processo, diminui o
   espaço de memória disponível no buraco e então aloca o novo processo entre esse espaço e o elemento
   anterior a ele. */
MemoryCase *allocProcessCase(processID id, numberOfSpaces size, MemoryCase *holeCaseThatFits, Memory *memory){
	MemoryCase *processCase;
	Hole *theHole;

	theHole =  (Hole *) holeCaseThatFits->holeOrProcess;
	processCase = createAProcessCase(id, theHole->begin, size, memory->lastProcess, nullMemoryCase());
	
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


/* Função que aloca um processo de tamanho exatamente igual ao do buraco. Nesse caso, nenhuma estrutura extra
   precisa ser criada, apenas precisamos sobrescrever o buraco com um processo. */

MemoryCase * overwriteHole(processID id, MemoryCase *holeCaseThatFits, Memory *memory){
	Hole *theHole;
	theHole =  (Hole *) holeCaseThatFits->holeOrProcess;
	
	if(theHole->prevHole)
		((Hole *)(theHole->prevHole->holeOrProcess))->nextHole = theHole->nextHole;
	if(theHole->nextHole)	
		((Hole *)(theHole->nextHole->holeOrProcess))->prevHole = theHole->prevHole;	

	holeCaseThatFits->type = process;
	holeCaseThatFits->holeOrProcess = (void *) initProcess(id, theHole->begin, theHole->available,
								memory->lastProcess, nullMemoryCase());

	if(memory->lastProcess)
		((Process *)(memory->lastProcess->holeOrProcess))->nextProcess = holeCaseThatFits;	
	memory->lastProcess = holeCaseThatFits;

	if(!memory->firstProcess)
		memory->firstProcess = holeCaseThatFits;

	return holeCaseThatFits;
}


/* Função que faz uma busca na lista de memória e retorna o buraco mais próximo de um objeto quando a
   lista é percorrida para a direita. Isso é importante no momento em que se exclui um processo da memória,
   já que precisamos saber se há a necessidade de mesclar o buraco criado com um imediatamente ao seu lado. */

MemoryCase * findNextHoleCase(MemoryCase *processCase){
	if (processCase == nullMemoryCase()) return nullMemoryCase();	
	else if(processCase->type == hole) return processCase;
	else return findNextHoleCase(processCase->next);
	
}

/* Função que faz uma busca na lista de memória e retorna o buraco mais próximo de um objeto quando a
   lista é percorrida para a esquera */

MemoryCase * findPrevHoleCase(MemoryCase *processCase){
	if (processCase == nullMemoryCase()) return nullMemoryCase();	
	else if(processCase->type == hole) return processCase;
	else return findPrevHoleCase(processCase->prev);
}


/* Função que seleciona o modo de exclusão de um processo, a partir da verificação dos buracos mais próximos
   a este. Temos quatro possibilidades, mesclar a direita, mesclar a esquerda, mesclar tanto para direita quanto
   para esquerda e, por último, não mesclar */

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


/* Função que exclui um processo da memória a partir do endereço de um nó e realiza o processo de
   mesclagem dos buracos subsequentes */

MemoryCase * endProcess(MemoryCase *processCase, MasterMemory *masterMemory){
	Memory *memory = masterMemory->memory;
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


/* Função que destrói um processo sem realizar verificação dos buracos ao seu redor */

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

/* Remove um segmento de processo da lista de processos e redefine os apontadores dos nós
   imediatamente ao seu lado */
MemoryCase * removeProcessOfProcessList(MemoryCase *processCase, Memory *memory){

	if(((Process *)(processCase->holeOrProcess))->prevProcess)
		((Process*)(((Process *)(processCase->holeOrProcess))->prevProcess->holeOrProcess))->nextProcess = 
		((Process *)(processCase->holeOrProcess))->nextProcess;
	else memory->firstProcess = ((Process *)(processCase->holeOrProcess))->nextProcess;

	if(((Process *)(processCase->holeOrProcess))->nextProcess)
		((Process*)(((Process *)(processCase->holeOrProcess))->nextProcess->holeOrProcess))->prevProcess = 
		((Process *)(processCase->holeOrProcess))->prevProcess;
	else memory->lastProcess = ((Process *)(processCase->holeOrProcess))->prevProcess;

	return processCase;
}

/* Função que realiza a união de dois buracos vizinhos */

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

/* Função que cria um elemento de memória do tipo buraco, 
   é utilizada apenas na inicialização do programa */

MemoryCase * makeInitialMemoryCase(void){
	MemoryCase *newMemoCase;
	newMemoCase = newMemoryCase();
	newMemoCase->type = hole;
	newMemoCase->holeOrProcess = (void *) makeInitialHole();
	newMemoCase->next = nullMemoryCase();
	newMemoCase->prev = nullMemoryCase();

	return newMemoCase;
}


/* Função que cria um elemento de memória do tipo processo já com um identificador,
   espaço que ocupa e onde começa na memória. */

MemoryCase * createAProcessCase(processID id, space begin, numberOfSpaces toUse, MemoryCase *prevProcessCase,
								   MemoryCase *nextProcessCase){
	MemoryCase *newProcessCase;
	newProcessCase = newMemoryCase();
	newProcessCase->type = process;
	newProcessCase->next = nullMemoryCase();
	newProcessCase->prev = nullMemoryCase();
	newProcessCase->holeOrProcess = (void *) initProcess(id, begin, toUse, 
							     prevProcessCase,
							     nextProcessCase);

	return newProcessCase;
}

/* Função que devolve um elemento de memória nulo */

MemoryCase *nullMemoryCase(void){
	return NULL;
}

/* Aloca um elemento de memória dinamicamente */ 

MemoryCase *newMemoryCase(void){
	return (MemoryCase *) malloc (sizeof(MemoryCase));
}

/*---------------------PROCESS FUNCTIONS----------------------*/

/* Função que inicia a estrutura mais simples de um processo, essa estrutura é uma
   das variáveis que um elemento de memória pode conter (process, hole). por meio
   de um ponteiro do tipo void declarado na criação de um elemento de memória,
   podemos alterar facilmente um elemento processo para um elemento buraco. */

Process *initProcess(processID id, space begin, numberOfSpaces inUse, MemoryCase *prevProcessCase,
							MemoryCase *nextProcessCase){
	Process * aProcess;
	aProcess = newProcess();
	aProcess->begin = begin;
	aProcess->inUse = inUse;
	aProcess->ID = id;
	aProcess->prevProcess = prevProcessCase;
	aProcess->nextProcess = nextProcessCase;

	return aProcess;
}

/* Aloca dinamicamente uma struct Process. */

Process *newProcess(void){
	return (Process *) malloc (sizeof(Process));
}

/* Gera um identificador único para novos processos */

processID newProcessID(void){
	static processID numberOfProcess = 0;
	return numberOfProcess++;
}
/*-----------------------HOLE FUNCTIONS-----------------------*/

/* Função que inicia a estrutura mais simples de um buraco, essa estrutura é uma
   das variáveis que um elemento de memória pode conter (process, hole). Por meio
   de um ponteiro do tipo void declarado na criação de um elemento de memória,
   podemos alterar facilmente um elemento processo para um elemento buraco. */

Hole *makeHole(space begin, numberOfSpaces available,MemoryCase *prevHole, MemoryCase *nextHole){
	Hole * aHole;
	aHole = newHole();
	aHole->begin = begin;
	aHole->available = available;
	aHole->prevHole = prevHole;
	aHole->nextHole = nextHole;

	return aHole;
}

/* Função que conduz de uma forma encapsulada a criação do primeiro elemento de
   memória ao se inicializar o programa */

Hole *makeInitialHole(void){
	return makeHole(0, SIZE_OF_ALL_SPACES, nullMemoryCase(), nullMemoryCase());
}

/* Retorna um ponteiro nulo Hole */

Hole *nullHole(void){
	return NULL;
}

/* Aloca dinamicamente uma struct Hole */
Hole *newHole(void){
	return (Hole *) malloc (sizeof(Hole));
}



















