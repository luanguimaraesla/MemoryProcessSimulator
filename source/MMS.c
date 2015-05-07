/* Memory Manager Simulator 
 * Created by Luan Guimarães and Vinicius Franco
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "mms_structures.h"
#include "mms_terminal_log_functions.h"
#include "mms_gui_generation.h"
#include "mms_creation_functions.h"
#include "mms_thread_args.h"
#include "mms_end_process_functions.h"
#include "mms_best_fit_insertion.h"
#include "mms_first_fit_insertion.h"
#include "mms_worst_fit_insertion.h"

pthread_mutex_t mutex_listModify;
pthread_cond_t cond_listModify;

void printInsertionModeSelectionMenu(void);
insertionMode getInsertionMode(void);
void printMemorySizeMenu(void);
numberOfSpaces getSize(void);
void ask_rcp_args(rcp_arg *args);

MemoryCase * addProcessFirstFit(numberOfSpaces size, priority index, Memory *memory);
MemoryCase * addProcessWorstFit(numberOfSpaces size, priority index, Memory *memory);
MemoryCase * addProcessBestFit(numberOfSpaces size, priority index, Memory *memory);
void * executeProcess(void *vargp);
void * randomCreateProcesses(void *vargp);
void * plotMemoryStatus(void *vargp);

/*-----------------------------MAIN-----------------------------*
 *                                                              *
 *    Função principal contendo a inicialização dos diferen-    *
 *    tes processos para a simulação de uma memória.            *
 *	                                                            *
 *    1. Inicialização da memória com um tamanho especificado   *
 *    pelo usuário.                                             *
 *    2. Inicialização dos parâmetros de entrada para a função  *
 *    geradora de processos, incluindo a determinação de qual   *
 *    tipo de inserção (first-fit, best-fit, worst-fit) será    *
 *    utilizada.                                                *
 *    3. Inicialização da variável mutex_listModify. Esta é     *
 *    responsável pelo controle de acesso à memória             *
 *    4. Cria as threads para criação de processos e atualiza-  *
 *    ção do frame de exibição.                                 *
 *    5. Espera até que todos os processos sejam criados e a    *
 *    thread criadora de processos seja encerrada.              *
 *    6. Aguarda para que processos ainda em execução sejam     *
 *    finalizados.                                              *
 *    7. Bloqueia a atualização do frame.                       *
 *    8. Aguarda até que a thread de atualização do frame seja  *
 *    encerrada.                                                *
 *    9. Destrói a fechadura da variável mutex_listModify.      *
 *	 10. Encerra a thread principal.                            *
 *                                                              *
 *--------------------------------------------------------------*/

int main(void){

	Memory *memory;
	rcp_arg args;
	pthread_t processesCreation, frameUpdate;
	bool frame_update = true;
	fu_arg frame_update_args;
	int memory_size;

	memory_size = getSize();
	memory = createMemory(memory_size);
	args.memory = memory;
	ask_rcp_args(&args);

	frame_update_args.memory = memory;
	frame_update_args.frame_update = &frame_update;
	frame_update_args.ui_params = createUI();
	frame_update_args.ui_params -> byte_size = ((double)MEMORY_UI_X/(double)memory_size);;
	
	pthread_mutex_init(&mutex_listModify, NULL);
	pthread_create(&processesCreation, NULL, randomCreateProcesses, &args);
	pthread_create(&frameUpdate, NULL, plotMemoryStatus, &frame_update_args);

	pthread_join(processesCreation, NULL);

	while(memory->firstProcessCase);
	sleep(1);
	frame_update = false;
	pthread_join(frameUpdate, NULL);
	pthread_mutex_destroy(&mutex_listModify);
	
	pthread_exit((void *)NULL);
	
	return 0;
}

/*-------------------------MENU FUNCTIONS-----------------------*
 *                                                              *
 *    Nesse trecho são declaradas as funções de interação com   *
 *    o usuário. São responsáveis por:                          *
 *    1. Perguntar e recuperar o tamanho da memória no início   *
 *    da execução de um programa.                               *
 *    2. Perguntar o tipo de inserção que o usuário deseja      *
 *    aplicar durante a execução do programa (first fit, best   *
 *    fit e worst fit).                                         *
 *    3. Perguntar e recuperar a quantidade de processos a se-  *
 *    rem criados, o tempo máximo entre a criação de cada pro-  *
 *    cesso, o tamanho máximo que um processo pode ter e quan-  *
 *    tos níveis de prioridade serão estabelecidos.             * 
 *                                                              *
 *--------------------------------------------------------------*/

void printInsertionModeSelectionMenu(void){
	printf("\nSelect the insertion mode: ");
	printf("\n1. First-Fit.\n2. Worst-Fit\n3. Best-Fit\n\nSelect: ");
}

insertionMode getInsertionMode(void){
	insertionMode option;
	printInsertionModeSelectionMenu();
	do{	
		scanf("%d", &option);
	}while(option < 1 && option > 3);
	return option;
}

void printMemorySizeMenu(void){
	printf("\nType the memory size: ");
}

numberOfSpaces getSize(void){
	numberOfSpaces size;
	printMemorySizeMenu();
	scanf("%lu", &size);
	return size;
}

void ask_rcp_args(rcp_arg *args){

	switch(getInsertionMode()){
		case 1:
			args->addProcessFunction = addProcessFirstFit;
			break;
		case 2:
			args->addProcessFunction = addProcessWorstFit;
			break;
		case 3:
			args->addProcessFunction = addProcessBestFit;
			break;
		default:
			printf("\nERROR!\n");
			exit(1); 
	}
	
	printf("\nHow many processes do you want to create? ");
	scanf("%u", &(args->numberOfProcesses));

	printf("Enter the maximum each process size: ");
	scanf("%lu", &(args->maxProcessSize));

	printf("Enter the maximum each process priority value: ");
	scanf("%u", &(args->maxPriorityIndex));

	printf("Enter the longest interval between creation processes: ");
	scanf("%u", &(args->maxProcessGenerateSleep));

}

MemoryCase *addProcessFirstFit(numberOfSpaces size, priority index, Memory *memory){
    /*------------------------------DESCRIÇÃO------------------------------*
     *                                                                     *
     *  Nessa função é inicialmente feita uma verificação para saber se    *
     *  o processo a ser inserido cabe na memória, se não couber, a ope-   * 
     *  ração e descartada e retorna um valor nulo.                        *
	 *  Se o processo couber, verifica-se, então, se o buraco encontrado   *
     *  na primeira posiçaõ é suficientemente grande para a alocação do    *
     *  processo. Se for, a função divideAndInsert é chamada, basicamente, * 
     *  essa função cria um processo entre o espaço e o nó anterior,       *
     *  redimensionando o buraco e alocando na memória o novo processo.    *
     *  Se não for, a função reallocAndInsert_best é chamada, basicamente, * 
     *  ela percorre a lista a partir do ponto de inserção escolhido e,    * 
     *  des locando processos, forma um espaço suficientemente grande pa-  *
     *  ra alocar o processo desejado.                                     *
     *  O processo permanece inativo na memória até que a thread 'execute' *
     *  seja criada, então o processo começa a ser executado.              *
     *                                                                     *
     *---------------------------------------------------------------------*/

	MemoryCase *firstHoleCase = memory->firstHoleCase;
	MemoryCase *response;
	execution_arg *exec_arg;

	if(memory->available < size || !firstHoleCase)
		return nullMemoryCase();

	if(size >= firstHoleCase->size)
		response = reallocAndInsert_best(size, index, firstHoleCase, memory);
	else
		response = divideAndInsert(size, index, firstHoleCase, memory);

	exec_arg = (execution_arg *) malloc (sizeof(execution_arg));
	exec_arg->selfCase = response;
	exec_arg->memory = memory;
	(memory->running)++;
	(memory->total)++;
	pthread_create(&(((Process *)(response->holeOrProcess))->execute), NULL, executeProcess, (void *)exec_arg );

	return response;
}

MemoryCase *addProcessWorstFit(numberOfSpaces size, priority index, Memory *memory){
    /*------------------------------DESCRIÇÃO------------------------------*
     *                                                                     *
     *  Nessa função é inicialmente feita uma verificação para saber se    *
     *  o processo a ser inserido cabe na memória, se não couber, a ope-   * 
     *  ração e descartada e retorna um valor nulo.                        *
	 *  Se o processo couber, verifica-se, então, se o buraco encontrado   *
     *  pela função acima é suficientemente grande para a alocação do      *
     *  processo. Se for, a função divideAndInsert é chamada, seu          *
     *  funcionamento será descrito a seguir, mas, basicamente, essa       *
     *  função cria um processo entre o espaço e o nó anterior, redimen-   *
     *  sionando o buraco e alocando na memória o novo processo.           *
     *  Se não for, a função reallocAndInsert_worst é chamada, seu fun-    *
     *  cionamento também será descrito a seguir, mas, basicamente, ela    *
     *  percorre a lista a partir do ponto de inserção escolhido e, des-   *
     *  locando processos, forma um espaço suficientemente grande para     *
     *  alocar o processo desejado.                                        *
     *  O processo permanece inativo na memória até que a thread 'execute' *
     *  seja criada, então o processo começa a ser executado.              *
     *                                                                     *
     *---------------------------------------------------------------------*/
	MemoryCase *worstFitHoleCase = findTheWorstFitHoleCase(size, memory);
	MemoryCase *response;
	execution_arg *exec_arg;

	if(memory->available < size || !worstFitHoleCase)
		return nullMemoryCase();
	if(size >= worstFitHoleCase->size)
		response = reallocAndInsert_worst(size, index, worstFitHoleCase, memory);
	else
		response = divideAndInsert(size, index, worstFitHoleCase, memory);

	exec_arg = (execution_arg *) malloc (sizeof(execution_arg));
	exec_arg->selfCase = response;
	exec_arg->memory = memory;
	(memory->running)++;
	(memory->total)++;
	pthread_create(&(((Process *)(response->holeOrProcess))->execute), NULL, executeProcess, (void *)exec_arg );

	return response;
}

MemoryCase *addProcessBestFit(numberOfSpaces size, priority index, Memory *memory){
    /*------------------------------DESCRIÇÃO------------------------------*
     *                                                                     *
     *  Nessa função é inicialmente feita uma verificação para saber se    *
     *  o processo a ser inserido cabe na memória, se não couber, a ope-   * 
     *  ração e descartada e retorna um valor nulo.                        *
	 *  Se o processo couber, verifica-se, então, se o buraco encontrado   *
     *  pela função acima é suficientemente grande para a alocação do      *
     *  processo. Se for, a função divideAndInsert é chamada, seu          *
     *  funcionamento será descrito a seguir, mas, basicamente, essa       *
     *  função cria um processo entre o espaço e o nó anterior, redimen-   *
     *  sionando o buraco e alocando na memória o novo processo.           *
     *  Se não for, a função reallocAndInsert_best é chamada, seu fun-     *
     *  cionamento já foi descrito acima, mas, basicamente, ela percorre   *
     *  a lista a partir do ponto de inserção escolhido e, deslocando      *
     *  processos, forma um espaço suficientemente grande para alocar o    *
     *  processo desejado.                                                 *
     *  O processo permanece inativo na memória até que a thread 'execute' *
     *  seja criada, então o processo começa a ser executado.              *
     *                                                                     *
     *---------------------------------------------------------------------*/	
	MemoryCase *bestFitHoleCase = findTheBestFitHoleCase(size, memory);
	MemoryCase *response;
	execution_arg *exec_arg;

	if(memory->available < size || !bestFitHoleCase)
		return nullMemoryCase();
	if(size >= bestFitHoleCase->size)
		response = reallocAndInsert_best(size, index, bestFitHoleCase, memory);
	else
		response = divideAndInsert(size, index, bestFitHoleCase, memory);

	exec_arg = (execution_arg *) malloc (sizeof(execution_arg));
	exec_arg->selfCase = response;
	exec_arg->memory = memory;
	(memory->running)++;
	(memory->total)++;
	pthread_create(&(((Process *)(response->holeOrProcess))->execute), NULL, executeProcess, (void *)exec_arg );

	return response;
}



void * executeProcess(void *vargp){
	/*-----------------------------------DESCRIÇÃO--------------------------------*
     *                                                                            *
     *  Função para a criação de uma thread, recebe uma struct definida no iní-   *
     *  cio do programa como parâmetro e a partir dela, é possível se executar    *
     *  com flexiblidade o pequeno algoritmo descrito.                            *
     *                                                                            *
     *  Essa função recebe como parâmetro uma struct que contém os dois únicos    *
     *  parâmetros necessários. O cabeçalho da memória e o endereço do processo   *
     *  que criou essa thread.                                                    *
     *                                                                            *
     *  Para simular a execução de um processo, na verdade, fazemos com que o     *
     *  processo aguarde um tempo randômico antes de ser finalizado. Esse tempo   *
     *  está entre 1 e MAX_PROCESS_TIME que é uma definição do sistema.           *
     *                                                                            *
     *  Após a espera desse tempo, o processo chama sua própria função de exclu-  *
     *  são e aguarda até que o acesso a lista seja liberado pela fechadura       *
     *  mutex_listModify. Então, encerra o processo e libera a fechadura.         *
     *                                                                            *
     *----------------------------------------------------------------------------*/
	execution_arg *exec_arg = (execution_arg *) vargp;	
	time_t t;
	srand((unsigned) time(&t));
	sleep(1 + rand() % (MAX_PROCESS_TIME - 1));

	pthread_mutex_lock(&mutex_listModify);
	endProcess(exec_arg->selfCase, exec_arg->memory);
	pthread_mutex_unlock(&mutex_listModify);

	pthread_exit((void *)NULL);
}

void * randomCreateProcesses(void *vargp){
	/*-----------------------------------DESCRIÇÃO--------------------------------*
     *                                                                            *
     *  A partir de uma série de parâmetros que são perguntados ao usuário, essa  *
     *  função cria automaticamente um número x de processos (digitados pelo usu- *
     *  ário).                                                                    *
     *  Dado um intervalo de prioridades e o máximo valor de tamanho, gera-se um  *
     *  processo com valores randômicos obedecendo esses intervalos.              *
     *  Já que temos três funções de criação de processos, há a necessidade de se *
     *  ter um ponteiro para receber qual função foi selecionada pelo usuário.    *
     *  Um tempo randômico que obedece um intervalo fornecido é forçado entre a   *
     *  criação de um e outro processo.                                           *
     *  A adição de um processo representa uma alteração na lista encadeada,      *
     *  para evitar conflitos e deadlocks, a fechadura para acesso da lista é     *
     *  utilizada.                                                                *
     *                                                                            *
     *----------------------------------------------------------------------------*/
	rcp_arg *args = (rcp_arg *) vargp;
	time_t t;
	MemoryCase *(*addProcessFunction)() = args->addProcessFunction;
	srand((unsigned) time(&t));
	for(;args->numberOfProcesses;(args->numberOfProcesses)--){
		pthread_mutex_lock(&mutex_listModify);
		(*addProcessFunction)(1 + rand() % (args->maxProcessSize - 1), rand() % args->maxPriorityIndex, args->memory);
		pthread_mutex_unlock(&mutex_listModify);
		sleep(rand() % args->maxProcessGenerateSleep);
	}

	pthread_exit((void *)NULL);
}

void * plotMemoryStatus(void *vargp){
    /*-----------------------------------DESCRIÇÃO--------------------------------*
     *                                                                            *
     *  Função da thread de atualização do frame.                                 *
     *                                                                            *
     *----------------------------------------------------------------------------*/
	fu_arg *args = (fu_arg *) vargp;
	
	while(*(args->frame_update)){
		pthread_mutex_lock(&mutex_listModify);
		printMemory(args->memory, args->ui_params);
		pthread_mutex_unlock(&mutex_listModify);
		sleep(1);
	}

	pthread_exit((void *)NULL);	
}









