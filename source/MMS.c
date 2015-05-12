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
#include "mms_save_pkg.h"

pthread_mutex_t mutex_listModify;
pthread_mutex_t mutex_play;
pthread_cond_t cond_play;
bool play = false;

void printInsertionModeSelectionMenu(void);
insertionMode getInsertionMode(void);
void printMemorySizeMenu(void);
numberOfSpaces getSize(void);
insertionMode ask_rcp_args(rcp_arg *args);
FILE* fileMenu(void);
insertionMode setInsertionMode(int insertionModeID, rcp_arg *args);
MemoryCase * addProcessFirstFit(numberOfSpaces size, priority index, Memory *memory);
MemoryCase * addProcessWorstFit(numberOfSpaces size, priority index, Memory *memory);
MemoryCase * addProcessBestFit(numberOfSpaces size, priority index, Memory *memory);
void * executeProcess(void *vargp);
void * randomCreateProcesses(void *vargp);
void * plotMemoryStatus(void *vargp);
void pauseSimulation(void);
void runSimulation(void);
int simulationMenu(void);
FILE *getFile(const char *str);

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
	FILE *archive;
	Memory *memory;
	rcp_arg args;
	pthread_t processesCreation, frameUpdate;
	bool frame_update = true;
	fu_arg frame_update_args;
	numberOfSpaces memory_size;
	insertionMode insertionModeID;

	archive = fileMenu();
	
	if(!archive){
		memory_size = getSize();
		memory = createMemory(memory_size);
		args.memory = memory;
		insertionModeID = ask_rcp_args(&args);
	}
	else{
		memory = dropSimulationFromFile(archive, &args, &insertionModeID);
		setInsertionMode(insertionModeID, &args);
		memory_size = memory->available + memory->inUse;
	}

	frame_update_args.memory = memory;
	frame_update_args.frame_update = &frame_update;
	frame_update_args.ui_params = createUI();
	frame_update_args.ui_params -> byte_size = ((double)MEMORY_UI_X/(double)memory_size);;
	
	pthread_mutex_init(&mutex_listModify, NULL);
	pthread_mutex_init(&mutex_play, NULL);
	pthread_cond_init(&cond_play, NULL);

	pthread_create(&processesCreation, NULL, randomCreateProcesses, &args);
	pthread_create(&frameUpdate, NULL, plotMemoryStatus, &frame_update_args);

	for(;;){
		switch(simulationMenu()){
			case 1:
				if(play){
					pauseSimulation();
					printf("\e[H\e[2J");
					printf("Simulation is paused!");
					printf("\n\n\n\n\n\n");
					sleep(2);
				}
				else{
					printf("\e[H\e[2J");
					printf("Sorry, it's already paused.");
					printf("\n\n\n\n\n\n");
					sleep(2);
				}
				break;
			case 2:
				if(!play){
					runSimulation();
					printf("\e[H\e[2J");
					printf("Simulation is running!");
					printf("\n\n\n\n\n\n");
					sleep(2);
				}
				else{
					printf("\e[H\e[2J");
					printf("Sorry, it's already running.");
					printf("\n\n\n\n\n\n");
					sleep(2);
				}
				break;
			case 3:
				if(play){
					pauseSimulation();
					printf("\e[H\e[2J");
					printf("Simulation is paused!");
					printf("\n\n\n\n\n\n");
					sleep(2);
				}
				printf("\e[H\e[2J");
				pushMemoryToFile(getFile("w"), &args, insertionModeID);
				printf("Done!");
				printf("\n\n\n\n\n\n");
				sleep(2);
				break;
			default:
				printf("\nEXITING...");
				printf("\n\n\n");
				frame_update = false;
				pthread_cond_destroy(&cond_play);
				pthread_mutex_destroy(&mutex_play);
				pthread_mutex_destroy(&mutex_listModify);
				return 0;
		}
	}

	frame_update = false;
	pthread_join(frameUpdate, NULL);
	pthread_join(processesCreation, NULL);
	pthread_exit((void *)NULL);
	return 1;
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

FILE* fileMenu(void){
	int option;
	printf("\e[H\e[2J");	
	printf("0. Exit.\n1. Run simulation from file.\n2. New simulation.\n\nSelect: ");
	do{
		scanf("%d", &option);
	}while(option < 0 || option > 2);
	
	if(!option)
		exit(0);
	else if (option == 1)
		return getFile("r");
	
	return NULL;
}

FILE *getFile(const char *str){
	char fileName[30];
	printf("Enter the file name: ");
	scanf("%s", fileName);
	return fopen(fileName, str);
}

int simulationMenu(void){
	int option;
	
	printf("\e[H\e[2J");
	printf("----> Memory Simulator Menu <----\n");
	printf("0. Exit.\n1. Pause\n2. Run\n3. Save\nSelect: ");
	do{
		scanf("%d", &option);
	}while(option < 0 || option > 3);
	
	return option;
}

void printInsertionModeSelectionMenu(void){
	printf("\nSelect the insertion mode: ");
	printf("\n1. First-Fit.\n2. Worst-Fit\n3. Best-Fit\n\nSelect: ");
}

insertionMode getInsertionMode(void){
	insertionMode option;
	printInsertionModeSelectionMenu();
	do{	
		scanf("%d", &option);
	}while(option < 1 || option > 3);
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

insertionMode setInsertionMode(int insertionModeID, rcp_arg *args){
	switch(insertionModeID){
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

	return insertionModeID;
}

insertionMode ask_rcp_args(rcp_arg *args){

	insertionMode mode = setInsertionMode(getInsertionMode(), args);
	
	printf("\nHow many processes do you want to create? ");
	scanf("%u", &(args->numberOfProcesses));

	printf("Enter the maximum each process size: ");
	scanf("%lu", &(args->maxProcessSize));

	printf("Enter the maximum each process priority value: ");
	scanf("%u", &(args->maxPriorityIndex));

	printf("Enter the longest interval between creation processes: ");
	scanf("%lu", &(args->maxProcessGenerateSleep));

	return mode;
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
	_program_time processTime;
	srand((unsigned) time(&t));
	processTime = 1 + rand() % (MAX_PROCESS_TIME - 1);

	while(processTime--){
		pthread_mutex_lock(&mutex_play);
		while(!play)
			pthread_cond_wait(&cond_play, &mutex_play);
		pthread_mutex_unlock(&mutex_play);		
		sleep(1);
	}

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

		pthread_mutex_lock(&mutex_play);
		while(!play)
			pthread_cond_wait(&cond_play, &mutex_play);
		pthread_mutex_unlock(&mutex_play);

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

void pauseSimulation(void){
	pthread_mutex_lock(&mutex_play);
	play = false;
	pthread_mutex_unlock(&mutex_play);
}

void runSimulation(void){
	pthread_mutex_lock(&mutex_play);
	play = true;
	pthread_cond_broadcast(&cond_play);
	pthread_mutex_unlock(&mutex_play);
}









