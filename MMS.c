/* Memory Manager Simulator */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <X11/Xlib.h>

#define DESTRUCT_WITHOUT_MERGE  0
#define DESTRUCT_MERGE_NEXT     1
#define DESTRUCT_MERGE_PREV     2
#define DESTRUCT_MERGE_BOTH     3
#define MAX_PROCESS_TIME 20
#define MEMORY_UI_X 600
#define MEMORY_UI_Y 180

#define GREEN "#00FF00"
#define BLACK "#00000f"
#define RED "#FFFFFF"

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

struct ExecProcessArg{
	struct Memory *memory;
	struct MemoryCase *selfCase;
};

struct RandomCreateProcessesArg{
	struct MemoryCase * (*addProcessFunction)(numberOfSpaces, priority, struct Memory *);
	unsigned int numberOfProcesses;
	numberOfSpaces maxProcessSize;
	priority maxPriorityIndex;
	unsigned int maxProcessGenerateSleep;
	struct Memory *memory;
};

struct FrameUpdateArg{
	bool * frame_update;
	struct Memory *memory;
	struct UIParameters *ui_params;
};

struct UIParameters{
	Display *dis;
	Window win;
	float byte_size;
};

typedef struct MemoryCase MemoryCase;
typedef struct Process Process;
typedef struct Hole Hole;
typedef struct Memory Memory;
typedef struct ExecProcessArg execution_arg;
typedef struct RandomCreateProcessesArg rcp_arg;
typedef struct FrameUpdateArg fu_arg;
typedef struct UIParameters ui_param;

/*-----------------------GLOBAL VARIABLES-----------------------*
 *                                                              *
 *     A utilização de variáveis globais é desencorajada pois   *
 *     acarreta em preocupações com modificações inconsistentes *
 *     já que são visíveis a todas funções.                     *
 *	   Entretanto, variáveis utilizadas para fechaduras (locks) *
 *     e semáforos devem ser acessíveis para todas funções      *
 *                                                              *
 *--------------------------------------------------------------*/


pthread_mutex_t mutex_listModify;
pthread_cond_t cond_listModify;

/*-----------------------FUNCTION PROTOTYPES--------------------*
 *                                                              *
 *    Seguem todos os protótipos das funções escritas neste     *
 *    programa. A utilização de protótipos visa uma melhor      *
 *    compreensão do conjunto de funções responsáveis por uma   *
 *    atividade específica.                                     *
 *                                                              *
 *--------------------------------------------------------------*/

/*1. ALLOC FUNCTIONS */
Memory * allocMemory(void);
MemoryCase * allocMemoryCase(void);
Hole * allocHole(void);
Process * allocProcess(void);

/*2. CREATE FUNCTIONS */
Memory * createMemory(numberOfSpaces size);
MemoryCase * createMemoryCase(memoryCaseType type, space begin, numberOfSpaces size,MemoryCase *next, MemoryCase *prev);
MemoryCase * createHoleCase(space begin, numberOfSpaces size, MemoryCase *nextHoleCase,
							MemoryCase *prevHoleCase, MemoryCase *next, MemoryCase *prev);
MemoryCase * createProcessCase(processID id, priority index, space begin, numberOfSpaces size,
							   MemoryCase *nextProcessCase, MemoryCase *prevProcessCase,
							   MemoryCase *next, MemoryCase *prev);
Hole * createHole(MemoryCase *nextHoleCase, MemoryCase *prevHoleCase);
Process * createProcess(processID id, priority index, MemoryCase *nextProcessCase, MemoryCase *prevProcessCase);
MemoryCase * createInitialHoleCase(numberOfSpaces size);

/*3. NULL FUNCTIONS */
MemoryCase * nullMemoryCase(void);

/*4. AUXILIAR FUNCTIONS */
processID getNewProcessID(void);

/*5. ADD PROCESS FUNCTIONS */
MemoryCase * addProcessFirstFit(numberOfSpaces size, priority index, Memory *memory);
MemoryCase * reallocAndInsert_best(numberOfSpaces size, priority index, MemoryCase *insertBegin, Memory *memory);
MemoryCase * overwriteHoleCase(priority index, MemoryCase *holeToOverwrite, Memory *memory);
MemoryCase * findNextPriorityListProcessCase(priority index, MemoryCase *firstProcessCase);
MemoryCase * divideAndInsert(numberOfSpaces size, priority index, MemoryCase * holeCaseToDivide, Memory *memory);
MemoryCase * findTheBestFitHoleCase(numberOfSpaces size, Memory *memory);
MemoryCase * reallocAndInsert_worst(numberOfSpaces size, priority index, MemoryCase *insertBegin, Memory *memory);
MemoryCase * findTheWorstFitHoleCase(numberOfSpaces size, Memory *memory);
MemoryCase * addProcessWorstFit(numberOfSpaces size, priority index, Memory *memory);
MemoryCase * addProcessBestFit(numberOfSpaces size, priority index, Memory *memory);
MemoryCase * findPrevPriorityListProcessCase(priority index, MemoryCase *firstProcessCase);
void removeHoleCase(MemoryCase *toRemove, Memory *memory);

/*6. PRINT FUNCTIONS */
void printMemory(Memory *memory, ui_param *ui_params);
void printProcessList(MemoryCase *firstProcessCase);
void printHoleList(MemoryCase *firstHoleCase);
void printMemoryTerminal(Memory *memory);

/*7. ENDP PROCESS FUNCTIONS */
MemoryCase * endProcess(MemoryCase *processCase, Memory *memory);
destructType selectDestructType(MemoryCase *processCase, MemoryCase *prevHoleCase, MemoryCase *nextHoleCase);
MemoryCase * findPrevHoleCase(MemoryCase *processCase, Memory *memory);
MemoryCase * findNextHoleCase(MemoryCase *processCase, Memory* memory);
MemoryCase * destructWithoutMerge(MemoryCase *processCase, MemoryCase *prevHoleCase, MemoryCase *nextHoleCase, Memory *memory);
MemoryCase * removeProcessOfProcessList(MemoryCase *processCase, Memory *memory);
MemoryCase * mergeHoleCases(MemoryCase *holeCaseA, MemoryCase *holeCaseB, Memory *memory);

/*8. MENU FUNCTIONS */
void printInsertionModeSelectionMenu(void);
insertionMode getInsertionMode(void);
void printMemorySizeMenu(void);
numberOfSpaces getSize(void);
void ask_rcp_args(rcp_arg *args);

/*9. THREADS FUNCTIONS */
void * executeProcess(void *vargp);
void * randomCreateProcesses(void *vargp);
void * plotMemoryStatus(void *vargp);

/*10. INTERFACE FUNCTIONS*/
ui_param * createUI();


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
	frame_update_args.ui_params -> byte_size = ((float)MEMORY_UI_X/(float)memory_size);;
	
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

/*------------------------PRINT FUNCTIONS-----------------------*
 *                                                              *
 *   Nessa parte estão implementadas todas as funções para      *
 *   a impressão e visualização instantânea da memória, da      *
 *   lista de processos e da lista de buracos.                  * 
 *                                                              *
 *--------------------------------------------------------------*/

void printMemory(Memory *memory, ui_param *ui_params){
	MemoryCase *firstCase = memory->begin;
	static int green_rect_count;
	static int black_rect_count;
	static char available[15], used[15], running[15], total[15];
	static XColor black_col, green_col, red_col;
	static Colormap colormap_black, colormap, colormap_red;
	static GC black_gc, green_gc, red_gc;
	static bool setColor = true;
	static float begin, size;
	static int end, end_2;
	static XRectangle r, r2;
	XRectangle *green_rects = (XRectangle *) malloc (sizeof(XRectangle) * memory->running);
	XRectangle *black_rects = (XRectangle *) malloc (sizeof(XRectangle) * memory->running);

	if(setColor){
		/*green gc*/
		colormap = DefaultColormap(ui_params->dis, 0);
		green_gc = XCreateGC(ui_params->dis, ui_params->win, 0, 0);
		XParseColor(ui_params->dis, colormap, GREEN, &green_col);
		XAllocColor(ui_params->dis, colormap, &green_col);
		XSetForeground(ui_params->dis, green_gc, green_col.pixel);

		/*black gc*/
		colormap_black = DefaultColormap(ui_params->dis, 0);
		black_gc = XCreateGC(ui_params->dis, ui_params->win, 0, 0);
		XParseColor(ui_params->dis, colormap_black, BLACK, &black_col);
		XAllocColor(ui_params->dis, colormap_black, &black_col);
		XSetForeground(ui_params->dis, black_gc, black_col.pixel);

		/*red gc*/
		colormap_red = DefaultColormap(ui_params->dis, 0);
		red_gc = XCreateGC(ui_params->dis, ui_params->win, 0, 0);
		XParseColor(ui_params->dis, colormap_red, RED, &red_col);
		XAllocColor(ui_params->dis, colormap_red, &red_col);
		XSetForeground(ui_params->dis, red_gc, red_col.pixel);
	
		setColor = false;
	}

	XDrawRectangle(ui_params->dis, ui_params->win, red_gc, 150, 10, 604, 180);
	XFlush(ui_params->dis);

	/*array de retangulos para serem inseridos*/
	green_rect_count = 0;
	black_rect_count = 0;

	do{
		if(firstCase->type == hole){
			begin = ((firstCase->begin)*(ui_params->byte_size));
			size = ((firstCase->size)*(ui_params->byte_size));
			if((MEMORY_UI_X - begin) < size){
				end = MEMORY_UI_X - begin;
				
				r.x = begin + 152;
				r.y = 12;
				r.width = end;
				r.height = 176;

				green_rects[green_rect_count] = r;
				green_rect_count++;

				end_2 = MEMORY_UI_X - (size + begin);

				r2.x = 152;
				r2.y = 12;
				r2.width = end_2;
				r2.height = 176;

				green_rects[green_rect_count] = r2;
				green_rect_count++;

			} else {

				r.x = begin + 152;
				r.y = 12;
				r.width = size;
				r.height = 176;

				green_rects[green_rect_count] = r;
				green_rect_count++;
			}
		}
		else{
			begin = ((firstCase->begin)*(ui_params->byte_size));
			size = ((firstCase->size)*(ui_params->byte_size));
			if((MEMORY_UI_X - begin) < size){
				end = MEMORY_UI_X - begin;
		
				r.x = begin + 152;
				r.y = 12;
				r.width = end;
				r.height = 176;

				black_rects[black_rect_count] = r;
				black_rect_count++;
				
				end_2 = size - begin;
				r2.x = 152;
				r2.y = 12;
				r2.width = end_2;
				r2.height = 176;

				black_rects[black_rect_count] = r2;
				black_rect_count++;

			} else {
				r.x = begin + 152;
				r.y = 12;
				r.width = size;
				r.height = 176;

				black_rects[black_rect_count] = r;
				black_rect_count++;

			}
		}

		/*insere os retangulos*/
		XFillRectangles(ui_params->dis, ui_params->win, black_gc, green_rects, green_rect_count);
		XFillRectangles(ui_params->dis, ui_params->win, green_gc, black_rects, black_rect_count);
		
		firstCase = firstCase->next;
	}while(firstCase != memory->begin);

	/*cria as strings*/
	sprintf(available, "%-14lu", memory->available);
	sprintf(used, "%-14lu", memory->inUse);
	sprintf(running, "%-14lu",memory->running);
	sprintf(total, "%-14lu",memory->total);

	/*limpa a area de string*/
	XClearArea(ui_params->dis, ui_params->win, 0,0, 140, 180, 0);

	/*desenha strings na tela.*/
	XDrawString(ui_params->dis, ui_params->win, green_gc, 10, 20, "Avaible: ", 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 70, 20, available, 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 10, 35, "Using:  ", 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 70, 35, used, 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 10, 50 , "Running:  ", 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 70, 50, running, 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 10, 75, "Total:  ", 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 70, 75, total, 8);
	XFlush(ui_params->dis);

	printMemoryTerminal(memory);
	free(green_rects);
	free(black_rects);

}

/*criar interface*/

ui_param * createUI(fu_arg *frame_update_args){
	Display *dis;
	Window win;
	ui_param *ui;
	dis = XOpenDisplay(NULL);
	win = XCreateSimpleWindow(dis, RootWindow(dis, 0), 1, 1, 800, 200, 0, BlackPixel (dis, 0), BlackPixel(dis, 0));
	XMapWindow(dis, win);
	XFlush(dis);
	ui = (ui_param*)malloc(sizeof(ui_param));
	ui->win = win;
	ui->dis = dis;
	return ui;
}

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

/*------------------------MEMORY FUNCTIONS----------------------*
 *                                                              *
 *   Nesta sessão, estão implementadas todas as funções rela-   *
 *   cionadas ao gerenciamento, criação e exclusão de proces-   *
 *   sos dentro da memória.                                     *
 *   Devido a complexissidade de alguns algoritmos presentes    *
 *   nesse trecho do código, cada função conterá uma explica-   *
 *   ção individual.                                            * 
 *                                                              *
 *--------------------------------------------------------------*/


Memory * allocMemory(void){
	/*---------------------------DESCRIÇÃO----------------------------*
     *                                                                *
     *	Aloca dinamicamente uma estrutura do time Memory.             *
     *                                                                *
     *----------------------------------------------------------------*/
	return (Memory *) malloc (sizeof(Memory));
}

Memory * createMemory(numberOfSpaces size){
	/*------------------------------DESCRIÇÃO----------------------------*
     *                                                                   *
     *	Inicializa uma memória preparando-a para o início do programa,   *
	 *  isto é:                                                          *
	 *  1. Cria um buraco com o tamanho definido pelo usuário;           *
     *  2. Seta como disponível todo esse espaço;                        *
     *  3. Seta como zero os espaços em uso (não há processos);          *
     *  4. Seta como zero a quantidade de processos sendo executados     *
     *     e a quantidade total de processos.                            *
     *  5. Atribui como início da lista encadeada da memória o buraco    *
     *     criado.                                                       *
     *                                                                   *
     *-------------------------------------------------------------------*/

	Memory *newMemory;

	newMemory = allocMemory();
	newMemory->available = size;
	newMemory->inUse = 0;
	newMemory->running = 0;
	newMemory->total = 0;
	newMemory->begin = createInitialHoleCase(size);
	newMemory->firstProcessCase = nullMemoryCase();
	newMemory->firstHoleCase = newMemory->begin;
	
	return newMemory;
}



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
				runner->begin = (currentNextSize - currentSizeAux) + runner->begin >= memory->available + memory->inUse ?
					 	 runner->begin + (currentNextSize - currentSizeAux) - memory->available - memory->inUse :
						 runner->begin + (currentNextSize - currentSizeAux);
				if(runner->size == 0)
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
				if(runner->size == 0)
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
				if(runner->size == 0)
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
				if(runner->size == 0)
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

/*FIRST FIT INSERTION*/

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

/*---------------------MEMORYCASE FUNCTIONS-----------------------*
 *                                                                *
 *   MemoryCase pode ser definida como a estrutura primordial     *
 *   que permite o funcionamento dessa simulação como uma lista.  *
 *   Essa estrutura é um simples nó de uma lista, porém, um pon-  *
 *   teiro do tipo void * permite que esse nó se torne um pro-    *
 *   cesso ou um buraco.                                          *
 *   Toda arquitetura desse software é baseada nessa estrutura    *
 *   e nas seguintes funções de criação e inicialização da mes-   *
 *   ma.                                                          *
 *                                                                *
 *----------------------------------------------------------------*/

MemoryCase * allocMemoryCase(void){
	/*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função que aloca dinamicamente um nó da lista encadeada.                     *
     *                                                                               *
     *-------------------------------------------------------------------------------*/
	return (MemoryCase *) malloc (sizeof(MemoryCase));
}

MemoryCase * createMemoryCase(memoryCaseType type, space begin, numberOfSpaces size,
							  MemoryCase *next, MemoryCase *prev){
	/*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função que inicializa um nó criado com os valores dos parâmetros.            *
     *                                                                               *
     *-------------------------------------------------------------------------------*/	
	MemoryCase *newMemoryCase;

	newMemoryCase = allocMemoryCase();
	newMemoryCase->type = type;
	newMemoryCase->begin = begin;
	newMemoryCase->size = size;
	newMemoryCase->next = next;
	newMemoryCase->prev = prev;

	return newMemoryCase;
}

MemoryCase * createProcessCase(processID id, priority index, space begin, numberOfSpaces size,
							   MemoryCase *nextProcessCase, MemoryCase *prevProcessCase,
							   MemoryCase *next, MemoryCase *prev){
    /*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função faz a mesma coisa que a anterior, porém é específica para a criação   *
     *  de um nó do tipo processo.                                                   *
     *                                                                               *
     *-------------------------------------------------------------------------------*/
	MemoryCase *newProcessCase;
	newProcessCase = createMemoryCase(process, begin, size, next, prev);
	newProcessCase->holeOrProcess = (void *) createProcess(id,index, nextProcessCase, prevProcessCase);

	return newProcessCase;
}

MemoryCase * createHoleCase(space begin, numberOfSpaces size, MemoryCase *nextHoleCase,
							MemoryCase *prevHoleCase, MemoryCase *next, MemoryCase *prev){
	/*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função faz a mesma coisa que a anterior, porém é específica para a criação   *
     *  de um nó do tipo buraco.                                                     *
     *                                                                               *
     *-------------------------------------------------------------------------------*/
	MemoryCase *newHoleCase;
	newHoleCase = createMemoryCase(hole, begin, size, next, prev);
	newHoleCase->holeOrProcess = (void *) createHole(nextHoleCase, prevHoleCase);
	return newHoleCase;
}

MemoryCase * createInitialHoleCase(numberOfSpaces size){
	MemoryCase * initialHoleCase;
	initialHoleCase = createHoleCase(0, size, nullMemoryCase(), nullMemoryCase(), 
											  nullMemoryCase(), nullMemoryCase());
	/*-----------------------------------DESCRIÇÃO-----------------------------------*
     *                                                                               *
     *  Função faz a mesma coisa que a anterior, porém é específica para a criação   *
     *  de um nó do tipo buraco para a inicialização da memória.                     *
     *                                                                               *
     *-------------------------------------------------------------------------------*/
	initialHoleCase->next = initialHoleCase;
	initialHoleCase->prev = initialHoleCase;
	((Hole *)(initialHoleCase->holeOrProcess))->nextHoleCase = initialHoleCase;
	((Hole *)(initialHoleCase->holeOrProcess))->prevHoleCase = initialHoleCase;
	return initialHoleCase;
}

MemoryCase * nullMemoryCase(void){
	/*-----------------------------------DESCRIÇÃO--------------------------------------*
     *                                                                                  *
     *  Função que retorna um nó nulo. Utilizada para melhorar a legibilidade do código.*
     *                                                                                  *
     *----------------------------------------------------------------------------------*/
	return NULL;
}

/*--------------------------HOLE FUNCTIONS------------------------*
 *                                                                *
 *   É a parte fundamental para a formação de uma MemoryCase do   *
 *   tipo hole.                                                   *
 *                                                                *
 *----------------------------------------------------------------*/

Hole * allocHole(void){
	/*-----------------------------------DESCRIÇÃO--------------------------------*
     *                                                                            *
     *  Função que aloca dinamicamente uma estrutura hole.                        *
     *                                                                            *
     *----------------------------------------------------------------------------*/
	return (Hole *) malloc (sizeof(Hole));
}

Hole * createHole(MemoryCase *nextHoleCase, MemoryCase *prevHoleCase){
	/*-----------------------------------DESCRIÇÃO--------------------------------*
     *                                                                            *
     *  Função que inicializa uma estrutura hole com seus buracos vizinhos.       *
     *                                                                            *
     *----------------------------------------------------------------------------*/
	Hole * newHole;
	
	newHole = allocHole();
	newHole->nextHoleCase = nextHoleCase;
	newHole->prevHoleCase = prevHoleCase;
	return newHole;
}

/*-----------------------PROCESS FUNCTIONS------------------------*
 *                                                                *
 *   É a parte fundamental para a formação de uma MemoryCase do   *
 *   tipo process.                                                *
 *                                                                *
 *----------------------------------------------------------------*/

Process * allocProcess(void){
	/*-----------------------------------DESCRIÇÃO--------------------------------*
     *                                                                            *
     *  Função que aloca dinamicamente uma estrutura process.                     *
     *                                                                            *
     *----------------------------------------------------------------------------*/
	return (Process *) malloc (sizeof(Process));
}

Process * createProcess(processID id, priority index, 
						MemoryCase *nextProcessCase, 
						MemoryCase *prevProcessCase){
	/*-----------------------------------DESCRIÇÃO--------------------------------*
     *                                                                            *
     *  Função que inicializa uma estrutura process com os valores dos parâmetros.*
     *                                                                            *
     *----------------------------------------------------------------------------*/
	Process *newProcess;

	newProcess = allocProcess();
	newProcess->id = id;
	newProcess->index = index;
	newProcess->nextProcessCase = nextProcessCase;
	newProcess->prevProcessCase = prevProcessCase;

	return newProcess;
}

processID getNewProcessID(void){
	/*-----------------------------------DESCRIÇÃO--------------------------------*
     *                                                                            *
     *  Gera um identificador único para um processo criado.                      *
     *                                                                            *
     *----------------------------------------------------------------------------*/
	static processID id = -1;
	return ++id;
}

/*-----------------------THREADS FUNCTIONS------------------------*
 *                                                                *
 *   Se os algoritmos de inserção são considerados os corações    *
 *   do programa, as threads podem ser tratadas como suas pernas, *
 *   pois, na verdade, são elas que vão permitir a dinâmica do    *
 *   código.                                                      *
 *                                                                *
 *----------------------------------------------------------------*/

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









