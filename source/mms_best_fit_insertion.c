#include "mms_best_fit_insertion.h"
#include "mms_first_fit_insertion.h"
#include "mms_creation_functions.h"
#include "mms_thread_args.h"
#include <math.h>
#include <stdlib.h>
/*----------------------BEST FIT INSERTION----------------------*
 *                                                              *
 *   As funções a seguir fazem parte do grupo responsável pela  *
 *   inserção no modo 'best fit'. Esse analiza a lista de bu-   *
 *   racos, escolhendo um cujo módulo da diferença entre seu    *
 *   tamanho e o tamanho do processo a ser incluído seja o      *
 *   menor possível entre os buracos.                           * 
 *                                                              *
 *--------------------------------------------------------------*/

MemoryCase *reallocAndInsert_best(numberOfSpaces size, priority index, MemoryCase *insertBegin, Memory *memory){
	/*--------------------------------DESCRIÇÃO--------------------------------*
     *                                                                         *
     *    Esse algoritmo é considerado um dos corações do programa. Ele é      *
     *    responsável por varrer a memória duplamente e circularmente enca-    *
     *    deada a partir de um nó aleatório, fazendo isso em ambos sentidos    *
     *    simultâneamente.                                                     *
     *    Essa função é chamada sempre que buraco escolhido para a inserção    *
     *    de um processo é menor do que o necessário para a alocação do mesmo, *
     *    ocasionando a necessidade do deslocamento dos processos imediata-    *
     *    mente vizinhos de forma com que esse espaço seja "aberto".           *
     *    O objetivo é encontrar, seguindo um algoritmo de busca estabelecido, *
     *    os limites do deslocamento para trás e para frente e só depois des-  *
     *    locar os processos até esses limites, criando um espaço "no meio" de *
     *    tamanho perfeito para a alocação do novo processo.                   *
     *    O seu funcionamento está melhor descrito nos cometários seguintes.   * 
     *                                                                         *
     *-------------------------------------------------------------------------*/

    /*--------------------------DESCRIÇÃO DO ALGORÍTMO---------------------------*
     *                                                                           *
     *   O parâmetro 'MemoryCase *inserBegin' já comporta o buraco de inserção   *
     *   previamente escolhido. Podemos, então, começar a verificação dos des-   *
     *   locamentos a partir dos buracos imediatamente vizinhos, para trás e     *
     *   para frente. 'currentPrevHoleCase' e 'currentNextHoleCase' são, então,  *
     *   setados com esses valores.                                              *
	 *	 O ponteiro 'runner' terá sua utilidade explicada adiante.               *
	 *	 A variáveis currentNextSize e currentPrevSize armazenam o número de     *
     *   posições deslocadas em relação ao início da busca (insertBegin). Já     *
     *   'currentSizeAux' armazena o valor total dos deslocamento, tanto para    *
     *   frente, quanto para trás.                                               *
     *                                                                           *
     *---------------------------------------------------------------------------*/

	MemoryCase *currentPrevHoleCase = ((Hole*)(insertBegin->holeOrProcess))->prevHoleCase;
	MemoryCase *currentNextHoleCase = ((Hole*)(insertBegin->holeOrProcess))->nextHoleCase;
	MemoryCase *runner;
	numberOfSpaces currentSizeAux = 0;
	numberOfSpaces currentPrevSize = 0;
	numberOfSpaces currentNextSize = 0;
	
	/*---------------------------DESCRIÇÃO DO ALGORÍTMO---------------------------*
     *                                                                            *
     *    O algoritmo a seguir busca os limites de deslocamento para trás e       *
     *    para frente dos processos vizinhos até que seja possível, deslocando    *
     *    esses processos até esses limites, inserir um novo processo de tama-    *
     *    nho 'size' entre eles. Para isso, percorre-se os buracos a partir de    *
     *    'currentPrevHoleCase' e 'currentNextHoleCase', que começam respecti-    *
     *    vamente nos buracos imediatamente vizinhos à esquerda e à direita na    *
     *    lista.                                                                  *
	 *    Compara-se o módulo da diferença entre a quantidade de espaços que      *
     *    restam ser deslocados e o tamanho do buraco a esquerda com o módulo     *
     *    dessa mesma diferença só que com o tamanho do buraco a direita,         *
     *    Então, por se tratar de uma inserção <best fit>, opta-se pelo caminho   *
     *    (nó) cujo módulo dessa diferença seja o menor, isto é, escolhe-se o     *
     *    buraco com o tamanho mais próximo ao que resta ser inserido.            *
	 *    Quando esse buraco é selecionado, o contador que armazena a quantida-   *
     *    de de espaços a serem deslocados para essa direção é incrementado do    *
     *    valor de seu tamanho e, então, o apontador do buraco selecionado passa  *
     *    a referenciar o próximo buraco da lista seguindo sua direção.           *
     *    Ou seja, caso 'currentPrevHoleCase' (apontador para buracos à esquerda) *
     *    seja o buraco selecionado, seu tamanho é adicionado a 'currentPrevSize' *
     *    (contador de espaços que serão deslocados para esquerda) e então esse   *
     *    ponteiro passa a referenciar o próximo buraco a esquerda.               *
     *    Esse processo se repete até que a soma dos contadores de espaço seja    *
     *    maior ou igual ao tamanho do processo a ser inserido menos o tamanho    *
     *    do espaço central (insertBegin).                                        *
     *                                                                            *
     *----------------------------------------------------------------------------*/
	while(insertBegin->size + currentPrevSize + currentNextSize < size){
		if(abs((size - (currentPrevSize + currentNextSize)) - currentPrevHoleCase->size) <
		   abs((size - (currentPrevSize + currentNextSize)) - currentNextHoleCase->size)){
			if(currentPrevHoleCase->size + currentNextSize + insertBegin->size + currentPrevSize >= size){
				currentPrevSize = (size - currentNextSize - insertBegin->size);
				currentPrevHoleCase = ((Hole *)(currentPrevHoleCase->holeOrProcess))->prevHoleCase;
				break;
			}
			else{
				currentPrevSize += currentPrevHoleCase->size;
				currentPrevHoleCase = ((Hole *)(currentPrevHoleCase->holeOrProcess))->prevHoleCase;
			}
		}
		else{
			if(currentNextHoleCase->size + currentNextSize + insertBegin->size + currentPrevSize >= size){
				currentNextSize = size - currentPrevSize - insertBegin->size;
				currentNextHoleCase = ((Hole *)(currentNextHoleCase->holeOrProcess))->nextHoleCase;
				break;
			}
			else{
				currentNextSize += currentNextHoleCase->size;
				currentNextHoleCase = ((Hole *)(currentNextHoleCase->holeOrProcess))->nextHoleCase;
			}		
		}
	}	

	/*---------------------------DESCRIÇÃO DO ALGORÍTMO---------------------------*
     *                                                                            *
     *    O algoritmo a seguir desloca os processos a direita até o limite esta-  *
     *    belecido no código descrito acima. O cálculo é simples, cada processo   *
     *    encontrado pelo apontador 'runner', que percorre a memória do nó à ime- *
     *    diatamente a direita do 'centro' (insertBegin) até o limite a direita,  *
     *    é deslocado para a direita segundo:                                     *
     *                                                                            *
     *                         P(k) = P(k-1) + (N - S)                            *
     *                                                                            *
     *    P = posição inicial;                                                    *
     *    k = relacional de tempo, no caso, P(k-1) é a posição anterior;          *
     *    N = tamanho que deve ser deslocado a direita;                           *
     *    S = soma dos tamanhos dos buracos encontrados entre o 'centro' e o li-  *
     *        mite a direita.                                                     *
     *                                                                            *
     *    Cada vez que runner encontra um buraco antes do limite, esse buraco é   *
     *    removido e seu tamanho é adicionado em S.                               *
     *                                                                            *
     *    Obs.: É importante observar que caso o valor do tamanho da memória seja *
     *    ultrapassado, por se tratar de uma lista circular, significa que o pro- *
     *    cesso deve continuar na posição inicial. Essa verificação e o possível  *
     *    ajuste de valores é implementada nos dois condicionais ternários abaixo.*
     *                                                                            *
     *                       P(k) = P(k-1) + (N - S) - T                          *
     *                                                                            *
     *    T = tamanho absoluto da memória.                                        *
     *----------------------------------------------------------------------------*/
	runner = insertBegin->next;
	while(runner != currentNextHoleCase && currentNextSize){
		if(runner->type == process){
			runner->begin = ((currentNextSize - currentSizeAux) + runner->begin) >= (memory->available + memory->inUse) ?
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
				runner->begin = ((currentNextSize - currentSizeAux) + runner->begin) >= (memory->available + memory->inUse) ?
					 	 runner->begin + (currentNextSize - currentSizeAux) - memory->available - memory->inUse :
						 runner->begin + (currentNextSize - currentSizeAux);
				if(runner->size <= 0)
					removeHoleCase(runner, memory);
				break;
			}
		}
		runner = runner->next;
	}

    /*---------------------------DESCRIÇÃO DO ALGORÍTMO---------------------------*
     *                                                                            *
     *    A ideia do próximo conjunto de instruções é a mesma do anterior, porém  *
     *    para a esquerda. Alguns ajustes devem ser feitos no cálculo, veja:      *
     *                                                                            *
     *                         P(k) = P(k-1) - (L - S)                            *
     *                                                                            *
     *    P = posição inicial;                                                    *
     *    k = relacional de tempo, no caso, P(k-1) é a posição anterior;          *
     *    L = tamanho que deve ser deslocado a esquerda;                          *
     *    S = soma dos tamanhos dos buracos encontrados entre o 'centro' e o li-  *
     *        mite a esquerda (começa em 0).                                      *
     *                                                                            *
     *    Cada vez que runner encontra um buraco antes do limite, esse buraco é   *
     *    removido e seu tamanho é adicionado em S da mesma forma como o deslo-   *    
     *    camento a direita.                                                      *
     *                                                                            *
     *    Obs.: Diferentemente do deslocamento a direita, a preocupação inerente  *
     *    se dá em relação ao estouro da posição zero, já que decrementações su-  *
     *    cessivas acarretariam em algum momento neste fato. A verificação é      *
     *    feita pelos operadores ternários a seguir e, caso haja um estouro,      *
     *    a posição pode ser determinada pelo seguinte cálculo:                   *
     *                                                                            *
     *                     P(k) = T - | P(k-1) - (L - S) |                        *
     *                                                                            *           
     *    T = tamanho absoluto da memória.                                        *
     *                                                                            *
     *----------------------------------------------------------------------------*/
	currentSizeAux = 0;
	runner = insertBegin->prev;
	while(runner != currentPrevHoleCase && currentPrevSize){
		if(runner->type == process){
			runner->begin = ((signed long)(runner->begin) - (signed long)(currentPrevSize - currentSizeAux) < 0) ?
					memory->available + memory->inUse - abs((signed long)(runner->begin) - (signed long)(currentPrevSize - currentSizeAux)) :
					runner->begin - (currentPrevSize - currentSizeAux);
		}		
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
	insertBegin->begin = (insertBegin->prev->size + insertBegin->prev->begin) >= memory->inUse + memory->available ?
						insertBegin->prev->size + insertBegin->prev->begin - memory->inUse - memory->available :
						insertBegin->prev->size + insertBegin->prev->begin;
	
	memory->available -= size;
	memory->inUse+=size;

	return overwriteHoleCase(index, insertBegin, memory);
}

MemoryCase *findTheBestFitHoleCase(numberOfSpaces size, Memory *memory){
	/*------------------------------DESCRIÇÃO----------------------------*
     *                                                                   *
     *  Função que percorre a toda a lista de buracos escolhendo um      *
     *  cujo módulo da diferença entre seu tamanho e o tamanho do pro-   *
     *  a ser incluído seja o menor possível entre os buracos            *
     *                                                                   *
     *-------------------------------------------------------------------*/

	MemoryCase * runner = memory->firstHoleCase;
	MemoryCase * bestFitHoleCase = runner;
	numberOfSpaces minDiference = runner->size;
	do{
		if(abs(runner->size - size) < minDiference){
			minDiference = abs(runner->size - size);
			bestFitHoleCase = runner;
		}
		runner = ((Hole *)(runner->holeOrProcess))->nextHoleCase;
	}while(runner != memory->firstHoleCase);
	return bestFitHoleCase;	
}
