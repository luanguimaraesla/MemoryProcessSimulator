#include <stdio.h>
#include <stdlib.h>

#define SIZE_OF_MEMORY 200
#define HOLE 0
#define PROCESS 1

/*-------------------------TYPEDEFS---------------------------*/

typedef int space;
typedef int numberOfSpaces;

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

	struct Hole *nextHole;
	struct Hole *prevHole;
};

struct Process{
	space begin;
	numberOfSpaces inUse;
};

struct Memory{
	struct MemoryCase *init;
	struct MemoryCase *final;
};

typedef struct MemoryCase MemoryCase;
typedef struct Hole Hole;
typedef struct Process Process;
typedef struct Memory Memory;

/*--------------------FUNCTIONS HEADERS-----------------------*/

/*---------------------------MAIN-----------------------------*/

int main(void){

	return 0;
}

/*---------------------------MENU-----------------------------*/

/*---------------------MEMORY FUNCTIONS-----------------------*/

/*-------------------MEMORY CASE FUNCTIONS--------------------*/

/*---------------------PROCESS FUNCTIONS----------------------*/

/*-----------------------HOLE FUNCTIONS-----------------------*/
