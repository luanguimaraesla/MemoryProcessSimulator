/* Memory Manager Simulator */

#include <stdio.h>
#include <stdlib.h>

/*------------------------TYPEDEFS-------------------------*/

typedef unsigned long space;
typedef unsigned long numberOfSpaces;
typedef unsigned long processID;
typedef unsigned long numberOfProcesses;

/*--------------------------ENUMS--------------------------*/

enum memoryElementType{
	hole, process
};

typedef enum memoryElementType memoryElementType;

/*-----------------------STRUCTS---------------------------*/

struct MemoryCase{
	memoryElementType type;	
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
	struct MemoryCase *nextProcessCase;
	struct MemoryCase *prevProcessCase;
};

struct Memory{
	numberOfProcesses total;
	numberOfProcesses running;
	numberOfSpaces available;
	numberOfSpaces inUse;
	struct MemoryCase *begin;
};

typedef struct MemoryCase MemoryCase;
typedef struct Process Process;
typedef struct Hole Hole;
typedef struct Memory Memory;

/*-------------------------MAIN----------------------------*/

int main(void){
	
	return 0;
}

/*----------------------FUNCTIONS--------------------------*/


