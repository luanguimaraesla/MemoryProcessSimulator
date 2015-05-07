#include "mms_best_fit_insertion.h"
#include "mms_first_fit_insertion.h"
#include "mms_creation_functions.h"
#include "mms_thread_args.h"
#include <math.h>
#include <stdlib.h>

MemoryCase *divideAndInsert(numberOfSpaces size, priority index, MemoryCase * holeCaseToDivide, Memory *memory){
	/*------------------------------DESCRIÇÃO------------------------------*
     *                                                                     *
     *    Essa função é chamada quando um buraco é suficientemente grande  *
     *    para conter um processo. Basicamente, ela reduz o espaço desse   *
     *    buraco e insere na lista encadeada um novo nó contendo um pro-   *
     *    cesso imediatamente entre o buraco e o nó anterior a ele.        *
     *                                                                     *
     *---------------------------------------------------------------------*/
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

void removeHoleCase(MemoryCase *toRemove, Memory *memory){
    /*------------------------------DESCRIÇÃO------------------------------*
     *                                                                     *
     *  Essa função remove o nó 'toRemove' da lista de buracos da memória  *
     *                                                                     *
     *---------------------------------------------------------------------*/
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
	/*------------------------------DESCRIÇÃO------------------------------*
     *                                                                     *
     *  Essa função sobrescreve um buraco com um processo, inserindo o nó  *
     *  na lista de processos, removendo-o da lista de buracos e mantendo  *
     *  as dimensões do buraco sobrescrito.                                *
     *  Otimiza, assim, inclusões quando o processo é exatamente do mesmo  *
     *  tamanho que o buraco, já que não precisremos redefinir os aponta-  *
     *  dores da lista principal.                                          *
     *                                                                     *
     *---------------------------------------------------------------------*/
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
	/*------------------------------DESCRIÇÃO-------------------------------*
     *                                                                      *
     *  Essa função pesquisa na lista de processos, de acordo com a prio-   *
     *  ridade de cada um, a posição imediata do primeiro nó de maior       *
     *  prioridade em relação ao valor passado como parâmetro.              *
     *  Por exemplo, se temos três processos com níveis de prioridade 1,    *
     *  5 e 7, se passarmos para o parâmetro 'index' o valor 3, será devol- *
     *  vido o segundo nó, cuja prioridade é 5.                             *
     *                                                                      *
     *----------------------------------------------------------------------*/
	if(!firstProcessCase) return nullMemoryCase();
	else if (((Process *)(firstProcessCase->holeOrProcess))->index >= index) return firstProcessCase;
	else return findNextPriorityListProcessCase(index, ((Process *)(firstProcessCase->holeOrProcess))->nextProcessCase);
}

MemoryCase *findPrevPriorityListProcessCase(priority index, MemoryCase *firstProcessCase){
	/*------------------------------DESCRIÇÃO-------------------------------*
     *                                                                      *
     *  Faz o mesmo que a função anterior, porém retorna a posição          *
     *  imediata do primeiro nó de prioridade inferior.                     *
     *                                                                      *
     *----------------------------------------------------------------------*/
	MemoryCase *prevProcessCase = nullMemoryCase();
	while(firstProcessCase && index > ((Process *)(firstProcessCase->holeOrProcess))->index){
			prevProcessCase = firstProcessCase;
			firstProcessCase = ((Process *)(firstProcessCase->holeOrProcess))->nextProcessCase;
	}
	return prevProcessCase;
}

