#include "mms_worst_fit_insertion.h"
#include "mms_first_fit_insertion.h"
#include "mms_creation_functions.h"
#include "mms_thread_args.h"
#include <math.h>
#include <stdlib.h>

/*---------------------WORST FIT INSERTION----------------------*
 *                                                              *
 *   As funções a seguir fazem parte do grupo responsável pela  *
 *   inserção no modo 'worst fit'. Esse analiza a lista de bu-  *
 *   racos, escolhendo um cujo módulo da diferença entre seu    *
 *   tamanho e o tamanho do processo a ser incluído seja o      *
 *   maior possível entre os buracos.                           * 
 *                                                              *
 *--------------------------------------------------------------*/


MemoryCase *findTheWorstFitHoleCase(numberOfSpaces size, Memory *memory){
    /*------------------------------DESCRIÇÃO----------------------------*
     *                                                                   *
     *  Função que percorre a toda a lista de buracos escolhendo um      *
     *  cujo módulo da diferença entre seu tamanho e o tamanho do pro-   *
     *  a ser incluído seja o maior possível entre os buracos./           *
     *                                                                   *
     *-------------------------------------------------------------------*/
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

MemoryCase *reallocAndInsert_worst(numberOfSpaces size, priority index, MemoryCase *insertBegin, Memory *memory){
    /*------------------------------DESCRIÇÃO------------------------------*
     *                                                                     *
     *    O funcionamento dessa função é praticamente igual ao da          *
     *    'reallocAndInsert_best' com apenas uma exceção: os módulos       *
     *    das diferenças de tamanho entre o espaço que resta ser inse-     *
     *    rido e os buracos a esquerda e a direita são comparados de       *
     *    forma diferente, optando pelo buraco cuja diferença de tamanho   *
     *    seja a maior possível.                                           *
     *                                                                     *
     *---------------------------------------------------------------------*/
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
				if(runner->size <= 0)
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
				if(runner->size <= 0)
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
