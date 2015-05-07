#include <stdio.h>
#include "mms_terminal_log_functions.h"

void printMemoryTerminal(Memory *memory){
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
			printf("Priority: %d\n", ((Process *)(firstCase->holeOrProcess))->index);
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
