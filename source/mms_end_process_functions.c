#include "mms_end_process_functions.h"
#include "mms_creation_functions.h"
#include <stdlib.h>

/*-----------------------REMOVE FUNCTIONS-----------------------*
 *                                                              *
 *   As funções a seguir fazem parte do grupo responsável pela  *
 *   remoção de um processo da memória e da lista de processos  *
 *                                                              *
 *--------------------------------------------------------------*/

MemoryCase * findNextHoleCase(MemoryCase *processCase, Memory* memory){
    /*--------------------------------DESCRIÇÃO---------------------------------*
     *                                                                          *
     *  Função que encontra o primeiro próximo buraco em relação a um processo. *
     *                                                                          *
     *--------------------------------------------------------------------------*/
	MemoryCase *runner = processCase->next;
	do{
		if(runner->type == hole)
			return runner;
		runner = runner->next;
	}while(runner != processCase);
	return nullMemoryCase();
}


MemoryCase * findPrevHoleCase(MemoryCase *processCase, Memory *memory){
    /*--------------------------------DESCRIÇÃO---------------------------------*
     *                                                                          *
     *  Função que encontra o primeiro anterior buraco em relação a um processo.*
     *                                                                          *
     *--------------------------------------------------------------------------*/
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
	/*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função que toma como parâmetros um processo a ser excluído e seus bura-      *
     *  cos vizinhos, selecionando o tipo de exclusão de processos que será          *
     *  efetuada. Quatro casos são possíveis:                                        *
	 *                                                                               *
     *  1. Destruir processos sem mesclar buracos: acontece quando um processo tem   *
     *  como vizinhos outros dois processos.                                         *
     *  2. Destruir e mesclar para esquerda: acontece quando existe um processo a    *
     *  direita e um buraco a esquerda do processo a ser excluído, ou seja, há a     *
     *  necessidade de se unir esses dois processos.                                 *
     *  3. Destruir e mesclar para direita: faz o mesmo que o anterior só que com    *
     *  um buraco a direita.                                                         *
     *  4. Destruir e mesclar para os dois lados: quando um processo excluído es-    *
     *  tá entre dois buracos.                                                       *
     *                                                                               *
     *-------------------------------------------------------------------------------*/	
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
	/*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função que encerra um processo.                                              *
     *  Primeiro, verifica se o nó passado é um processo, se não for, retorna NULL,  *
     *  depois, faz as alterações no cabeçalho dizendo que está prestes a excluir um *
     *  processo. Então, um tipo de destruição de processos é selecionado com a fun- *
     *  ção descrita acima.                                                          *
     *  Depois, a função 'destructWithoutMerge é chamada, transformando um processo  *
     *  em um buraco sem realizar nenhum tipo de verificação dos seus vizinhos.      *
     *  Essa verificação já havia sido feita pela função selectDestructType. Então   *
     *  o switch verifica se há a necessidade de se mesclar buracos e executa o      *
     *  procedimento correto.                                                        *
     *                                                                               *
     *-------------------------------------------------------------------------------*/
	MemoryCase *nextHoleCase, *prevHoleCase, *newHoleCase;
	destructType option;
	
	if(processCase->type != process)
		return nullMemoryCase();
	else{
		memory->available += processCase->size;
		memory->inUse-= processCase->size;
	}
	
	(memory->running)--;

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
    /*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função que destrói um processo sobrescrevendo-o com um buraco. Entretanto,   *
     *  não faz a verificação dos buracos vizinhos para saber se há a necessidade    *
     *  de se mesclar.                                                               *
     *                                                                               *
     *-------------------------------------------------------------------------------*/
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
    /*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Dado um processo prestes a ser finalizado, essa função remove esse processo  *
     *  da lista de processos.                                                       *
     *                                                                               *
     *-------------------------------------------------------------------------------*/
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
	/*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função que mescla dois buracos consecutivos, excluindo da lista encadeada o  *
     *  primeiro e adicionando seu tamanho ao segundo.                               *
     *                                                                               *
     *-------------------------------------------------------------------------------*/
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


