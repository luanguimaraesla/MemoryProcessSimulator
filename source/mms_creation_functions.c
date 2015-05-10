#include "mms_creation_functions.h"
#include <stdlib.h>


Memory * allocMemory(void){
	/*---------------------------DESCRIÇÃO----------------------------*
     *                                                                *
     *	Aloca dinamicamente uma estrutura do time Memory.             *
     *                                                                *
     *----------------------------------------------------------------*/
	return (Memory *) malloc (sizeof(Memory));
}


Memory *createAndSetMemory(numberOfSpaces available, numberOfSpaces inUse, numberOfProcesses running,
			   numberOfProcesses total, MemoryCase *begin, MemoryCase *firstProcessCase,
			   MemoryCase *firstHoleCase){
	Memory *memory;

	memory = allocMemory();
	memory->available = available;
	memory->inUse = inUse;
	memory->running = running;
	memory->total = total;
	memory->begin = begin;
	memory->firstProcessCase = firstProcessCase;
	memory->firstHoleCase = firstHoleCase;
	
	return memory;
}

Memory * createMemory(numberOfSpaces size){
	/*------------------------------DESCRIÇÃO----------------------------*
     *                                                                   *
     *	Inicializa uma memória preparando-a para o início do programa,   *
	 *  isto é:                                                          *
	 *  1. Cria um buraco com o tamanho definido pelo usuário;           *
     *  2. Seta como disponível todo esse espaço;                        *
     *  3. Seta como zero os espaços em uso (não há processos);          *
     *  4. Seta como zero a quantidade de processos sendo executados     *
     *     e a quantidade total de processos.                            *
     *  5. Atribui como início da lista encadeada da memória o buraco    *
     *     criado.                                                       *
     *                                                                   *
     *-------------------------------------------------------------------*/

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

/*---------------------MEMORYCASE FUNCTIONS-----------------------*
 *                                                                *
 *   MemoryCase pode ser definida como a estrutura primordial     *
 *   que permite o funcionamento dessa simulação como uma lista.  *
 *   Essa estrutura é um simples nó de uma lista, porém, um pon-  *
 *   teiro do tipo void * permite que esse nó se torne um pro-    *
 *   cesso ou um buraco.                                          *
 *   Toda arquitetura desse software é baseada nessa estrutura    *
 *   e nas seguintes funções de criação e inicialização da mes-   *
 *   ma.                                                          *
 *                                                                *
 *----------------------------------------------------------------*/

MemoryCase * allocMemoryCase(void){
	/*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função que aloca dinamicamente um nó da lista encadeada.                     *
     *                                                                               *
     *-------------------------------------------------------------------------------*/
	return (MemoryCase *) malloc (sizeof(MemoryCase));
}


MemoryCase * createMemoryCase(memoryCaseType type, space begin, numberOfSpaces size,
							  MemoryCase *next, MemoryCase *prev){
	/*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função que inicializa um nó criado com os valores dos parâmetros.            *
     *                                                                               *
     *-------------------------------------------------------------------------------*/	
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
    /*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função faz a mesma coisa que a anterior, porém é específica para a criação   *
     *  de um nó do tipo processo.                                                   *
     *                                                                               *
     *-------------------------------------------------------------------------------*/
	MemoryCase *newProcessCase;
	newProcessCase = createMemoryCase(process, begin, size, next, prev);
	newProcessCase->holeOrProcess = (void *) createProcess(id,index, nextProcessCase, prevProcessCase);

	return newProcessCase;
}

MemoryCase * createHoleCase(space begin, numberOfSpaces size, MemoryCase *nextHoleCase,
							MemoryCase *prevHoleCase, MemoryCase *next, MemoryCase *prev){
	/*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função faz a mesma coisa que a anterior, porém é específica para a criação   *
     *  de um nó do tipo buraco.                                                     *
     *                                                                               *
     *-------------------------------------------------------------------------------*/
	MemoryCase *newHoleCase;
	newHoleCase = createMemoryCase(hole, begin, size, next, prev);
	newHoleCase->holeOrProcess = (void *) createHole(nextHoleCase, prevHoleCase);
	return newHoleCase;
}

MemoryCase * createInitialHoleCase(numberOfSpaces size){
	MemoryCase * initialHoleCase;
	initialHoleCase = createHoleCase(0, size, nullMemoryCase(), nullMemoryCase(), 
											  nullMemoryCase(), nullMemoryCase());
	/*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função faz a mesma coisa que a anterior, porém é específica para a criação   *
     *  de um nó do tipo buraco para a inicialização da memória.                     *
     *                                                                               *
     *-------------------------------------------------------------------------------*/
	initialHoleCase->next = initialHoleCase;
	initialHoleCase->prev = initialHoleCase;
	((Hole *)(initialHoleCase->holeOrProcess))->nextHoleCase = initialHoleCase;
	((Hole *)(initialHoleCase->holeOrProcess))->prevHoleCase = initialHoleCase;
	return initialHoleCase;
}

MemoryCase * nullMemoryCase(void){
	/*-----------------------------------DESCRIÇÃO--------------------------------------*
     *                                                                                  *
     *  Função que retorna um nó nulo. Utilizada para melhorar a legibilidade do código.*
     *                                                                                  *
     *----------------------------------------------------------------------------------*/
	return NULL;
}

/*--------------------------HOLE FUNCTIONS------------------------*
 *                                                                *
 *   É a parte fundamental para a formação de uma MemoryCase do   *
 *   tipo hole.                                                   *
 *                                                                *
 *----------------------------------------------------------------*/

Hole * allocHole(void){
	/*-----------------------------------DESCRIÇÃO--------------------------------*
     *                                                                            *
     *  Função que aloca dinamicamente uma estrutura hole.                        *
     *                                                                            *
     *----------------------------------------------------------------------------*/
	return (Hole *) malloc (sizeof(Hole));
}

Hole * createHole(MemoryCase *nextHoleCase, MemoryCase *prevHoleCase){
	/*-----------------------------------DESCRIÇÃO--------------------------------*
     *                                                                            *
     *  Função que inicializa uma estrutura hole com seus buracos vizinhos.       *
     *                                                                            *
     *----------------------------------------------------------------------------*/
	Hole * newHole;
	
	newHole = allocHole();
	newHole->nextHoleCase = nextHoleCase;
	newHole->prevHoleCase = prevHoleCase;
	return newHole;
}

/*-----------------------PROCESS FUNCTIONS------------------------*
 *                                                                *
 *   É a parte fundamental para a formação de uma MemoryCase do   *
 *   tipo process.                                                *
 *                                                                *
 *----------------------------------------------------------------*/

Process * allocProcess(void){
	/*-----------------------------------DESCRIÇÃO--------------------------------*
     *                                                                            *
     *  Função que aloca dinamicamente uma estrutura process.                     *
     *                                                                            *
     *----------------------------------------------------------------------------*/
	return (Process *) malloc (sizeof(Process));
}

Process * createProcess(processID id, priority index, 
						MemoryCase *nextProcessCase, 
						MemoryCase *prevProcessCase){
	/*-----------------------------------DESCRIÇÃO--------------------------------*
     *                                                                            *
     *  Função que inicializa uma estrutura process com os valores dos parâmetros.*
     *                                                                            *
     *----------------------------------------------------------------------------*/
	Process *newProcess;

	newProcess = allocProcess();
	newProcess->id = id;
	newProcess->index = index;
	newProcess->nextProcessCase = nextProcessCase;
	newProcess->prevProcessCase = prevProcessCase;

	return newProcess;
}

processID getNewProcessID(void){
	/*-----------------------------------DESCRIÇÃO--------------------------------*
     *                                                                            *
     *  Gera um identificador único para um processo criado.                      *
     *                                                                            *
     *----------------------------------------------------------------------------*/
	static processID id = -1;
	return ++id;
}
