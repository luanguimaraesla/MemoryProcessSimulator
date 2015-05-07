#ifndef MMS_STRUCTURES
#define MMS_STRUCTURES
#include <X11/Xlib.h>
#include <pthread.h>

#define MAX_PROCESS_TIME 20

/*--------------------------TYPEDEFS----------------------------*
 *                                                              *
 *	Para uma melhor compreensão e, levando em consideração      *
 *	a manutensibilidade e flexibilidade do código, optou-se     *
 *	por utilizar nomes alternativos para cada tipo de dado      *
 *	presente no sistema.                                        *
 *                                                              *
 *--------------------------------------------------------------*/

typedef unsigned int priority;
typedef unsigned long space;
typedef unsigned long numberOfSpaces;
typedef unsigned long processID;
typedef unsigned long numberOfProcesses;
typedef int destructType;
typedef int insertionMode;

/*----------------------------ENUMS-----------------------------*
 *                                                              *
 *	Enumerações foram utilizadas para manter a coerência        *
 *  das estruturas, já que, limitando o número de dados de      *
 *	determinado tipo, possíveis erros de atribuições incon-     *
 *	sistentes serão evitados ou mais facilmente detectados.     *
 *                                                              *
 *--------------------------------------------------------------*/

enum bool{
	false, true
};

enum memoryCaseType{
	hole, process
};

typedef enum memoryCaseType memoryCaseType;
typedef enum bool bool;

/*---------------------------STRUCTS----------------------------*
 *                                                              *
 *	Foram criadas estruturas que se dividem em quatro grupos:   *
 *                                                              * 
 *  1. Elementares: conjunto de estruturas que definem um nó    *
 *                  da lista encadeada.                         *
 *                                                              *
 *  2. Cabeçalho: responsável pelo gerenciamento do conjunto    *
 *                de nós existentes na lista encadeada.         *
 *                                                              *
 *  3. Argumentos: para a utilização da função criadora de      *
 *                 threads, foi necessária a criação de es-     *
 *                 truturas que agregassem os parâmetros das    *
 *                 funções executadas por cada thread.          *
 *                                                              *
 *--------------------------------------------------------------*/

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


typedef struct MemoryCase MemoryCase;
typedef struct Process Process;
typedef struct Hole Hole;
typedef struct Memory Memory;

#endif
