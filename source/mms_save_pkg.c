#include "mms_save_pkg.h"
#include "mms_creation_functions.h"
#include "mms_best_fit_insertion.h"
#include "mms_worst_fit_insertion.h"
#include "mms_first_fit_insertion.h"
#include <stdlib.h>

/*  Header:
	<numero_de_casas> <available> <inUse> <running> <total> <insertionMode> <toCreate> <maxSize> <maxPriority> <maxTimeCreation>

	Process:
		<identificador> <id> <prioridade> <begin> <size>
	Hole:
		<identificador> <begin> <size>
*/

unsigned long getFileInt(FILE *arq){
	char c;
	unsigned long sum = 0;
	c = fgetc(arq);
	if (c == ' ')
		c = fgetc(arq);
	
	while(c != ' ' && c != '\n'){
		if(c == EOF) return -1;
		sum = sum * 10 + (int)(c - '0');
		c = fgetc(arq);
	}
	return sum;
}

char getFileChar(FILE *arq){
	char c;
	c = fgetc(arq);
	if(c == ' ' || c == '\n')
		c = fgetc(arq);

	return c;
}

unsigned int getNumberOfMemoryCases(Memory *memory){
	MemoryCase *runner = memory->begin;
	unsigned int counter = 0;	
	
	do{
		counter++;
		runner = runner->next;
	}while(runner != memory->begin);
	return counter;
}

void pushMemoryToFile(FILE *arq, rcp_arg *keepCurrentArgs, insertionMode *functionID){
	Memory* memory = keepCurrentArgs->memory;	

	writeFileInt(getNumberOfMemoryCases(memory), arq);
	writeFileInt(memory->available, arq);
	writeFileInt(memory->inUse, arq);
	writeFileInt(memory->running, arq);
	writeFileInt(memory->total, arq);
	writeFileInt(*functionID, arq);
	writeFileInt(keepCurrentArgs->numberOfProcesses, arq);
	writeFileInt(keepCurrentArgs->maxProcessSize, arq);
	writeFileInt(keepCurrentArgs->maxPriorityIndex, arq);
	writeFileInt(keepCurrentArgs->maxProcessGenerateSleep, arq);
	

}

void writeFileInt(unsigned long number, FILE *arq){
	fprintf(arq, "%lu ", number);	
}

void writeFileChar(char letter, FILE *arq){
	fprintf(arq, "%c ", letter);
}

Memory *dropSimulationFromFile(FILE *arq, rcp_arg *keepOldArgs, insertionMode *functionID){
	Memory *memory;
	MemoryCase *prevCase = nullMemoryCase();
	MemoryCase *prevHoleCase = nullMemoryCase();
	MemoryCase *prevProcessCase = nullMemoryCase();	
	unsigned int numberOfCases;
	numberOfSpaces available, inUse, size, maxSize;
	insertionMode mode;
	numberOfProcesses running, total, toCreate;
	_program_time maxTimeCreation;
	priority index, maxPriority;
	processID id;
	space begin;
	
	numberOfCases = (unsigned int) getFileInt(arq);
	available = (numberOfSpaces) getFileInt(arq);
	inUse = (numberOfSpaces) getFileInt(arq);
	running = (numberOfProcesses) getFileInt(arq);
	total = (numberOfProcesses) getFileInt(arq);
	mode = (insertionMode) getFileInt(arq);
	toCreate = (numberOfProcesses) getFileInt(arq);
	maxSize = (numberOfSpaces) getFileInt(arq);
	maxPriority = (priority) getFileInt(arq);
	maxTimeCreation = (_program_time) getFileInt(arq);

	memory = createAndSetMemory(available, inUse, running, total, nullMemoryCase(), nullMemoryCase(), nullMemoryCase());

	keepOldArgs->numberOfProcesses = toCreate;
	keepOldArgs->maxProcessSize = maxSize;
	keepOldArgs->maxPriorityIndex = maxPriority;
	keepOldArgs->maxProcessGenerateSleep = maxTimeCreation;
	keepOldArgs->memory = memory;
	*functionID = mode;
	
	
	while(numberOfCases--){
		switch(getFileChar(arq)){
			case 'H':
				begin = (space) getFileInt(arq);
				size = (numberOfSpaces) getFileInt(arq);
				
				prevCase = prevHoleCase = createHoleCase(begin, size, nullMemoryCase(),
										  prevHoleCase, memory->begin, prevCase);

				if(memory->firstHoleCase == nullMemoryCase())
					memory->firstHoleCase = prevCase;
				if (memory->begin == nullMemoryCase())
					memory->begin = prevCase;

				((Hole *)(prevCase->holeOrProcess))->nextHoleCase = memory->firstHoleCase;
				((Hole *)(memory->firstHoleCase->holeOrProcess))->prevHoleCase = prevCase;
				memory->begin->prev = prevCase;
				break;
			case 'P':
				id = (processID) getFileInt(arq);
			    index = (priority) getFileInt(arq);
				begin = (space) getFileInt(arq);
				size = (numberOfSpaces) getFileInt(arq);

				prevCase = prevProcessCase = createProcessCase(id, index, begin, size, nullMemoryCase(),
											 prevProcessCase, memory->begin, prevCase);

				if(memory->firstProcessCase == nullMemoryCase())
					memory->firstProcessCase = prevCase;
				if(memory->begin == nullMemoryCase())
					memory->begin = prevCase;

				((Process *)(prevCase->holeOrProcess))->nextProcessCase = memory->firstProcessCase;
				((Process *)(memory->firstProcessCase->holeOrProcess))->prevProcessCase = prevCase;
				memory->begin->prev = prevCase;
				break;
			default:
				printf("FATAL ERROR!");
				exit(1);
		}
	}

	sintetizeMemoryPointers(memory);
	sintetizeHoleCasePointers(memory);
	sintetizeProcessCasePointers(memory);
	
	return memory;
}

void sintetizeMemoryPointers(Memory *memory){
	MemoryCase *runner = memory->begin->prev;

	while(runner != memory->begin){
		runner->prev->next = runner;
		runner = runner->prev;
	}
}

void sintetizeHoleCasePointers(Memory *memory){
	MemoryCase *runner = ((Hole *)(memory->firstHoleCase->holeOrProcess))->prevHoleCase;
	
	while(runner != memory->firstHoleCase){
		((Hole *)(((Hole *)(runner->holeOrProcess))->prevHoleCase->holeOrProcess))->nextHoleCase = runner;
		runner = ((Hole *)(runner->holeOrProcess))->prevHoleCase; 
	}
	
}

void sintetizeProcessCasePointers(Memory *memory){
	MemoryCase *runner = ((Process *)(memory->firstProcessCase->holeOrProcess))->prevProcessCase;
	while (runner != memory->firstProcessCase){
		((Process *)(((Process *)(runner->holeOrProcess))->prevProcessCase->holeOrProcess))->nextProcessCase = runner;
		runner = ((Process *)(runner->holeOrProcess))->prevProcessCase;
	}
	bubbleSortForProcessPriority(memory);
}

void bubbleSortForProcessPriority(Memory *memory){
	MemoryCase *runner = memory->firstProcessCase;
	MemoryCase *limit = ((Process *)(runner->holeOrProcess))->prevProcessCase;
	MemoryCase *nextAux;
	MemoryCase *prevAux;

	((Process *)(((Process *)(memory->firstProcessCase->holeOrProcess))->prevProcessCase->holeOrProcess))->nextProcessCase = nullMemoryCase();
	((Process *)(memory->firstProcessCase->holeOrProcess))->prevProcessCase = nullMemoryCase();

	for(;limit; limit = ((Process *)(limit->holeOrProcess))->prevProcessCase){
		runner = memory->firstProcessCase;
		while(runner != limit && runner){
			if ( ((Process *)(runner->holeOrProcess))->index > 
				 ((Process *)(((Process *)(runner->holeOrProcess))->nextProcessCase->holeOrProcess))->index){
				
				nextAux = ((Process *)(runner->holeOrProcess))->nextProcessCase;
				prevAux = ((Process *)(runner->holeOrProcess))->prevProcessCase;

				if(((Process *)(nextAux->holeOrProcess))->nextProcessCase)
					((Process *)(((Process *)(nextAux->holeOrProcess))->nextProcessCase->holeOrProcess))->prevProcessCase = runner;				
			
				((Process *)(runner->holeOrProcess))->nextProcessCase = ((Process *)(nextAux->holeOrProcess))->nextProcessCase;
				((Process *)(runner->holeOrProcess))->prevProcessCase = nextAux;

				((Process *)(nextAux->holeOrProcess))->prevProcessCase = prevAux;
				((Process *)(nextAux->holeOrProcess))->nextProcessCase = runner;

				if(prevAux)
					((Process *)(prevAux->holeOrProcess))->nextProcessCase = nextAux;

				if(runner == memory->firstProcessCase)
					memory->firstProcessCase = nextAux;

				if(nextAux == limit){
					runner = limit;
					continue;
				}
			}
			else{
				runner = ((Process *)(runner->holeOrProcess))->nextProcessCase;
			}
		}
	}
}

/*-----------------------------------------------------------------------------------------------------------------------*/













