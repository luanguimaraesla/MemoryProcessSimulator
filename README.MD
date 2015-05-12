# Simulador de Memória com Listas Encadeadas

Esse programa foi desenvolvido com a finalidade de simular, utilizando listas encadeadas, o funcionamento de uma memória em ambiente linux/gcc.

## Como Compilar e Executar

Para compilar e executar o programa em um sistema operacional Linux, siga as seguintes instruções:

* Abra o terminal;
* Encontre o diretório raiz do projeto;
* Limpe os arquivos objeto:
	**$ make clean** 
* Compile o programa: 
	**$ make**
* Execute:
	**$ make run**

## Funcionamento

O funcionamento do programa está descrito a seguir:

### Nova simulação

* Escolha, entre as opções de execução, uma nova simulação.
* Insira o tamanho da memória que deseja criar em bytes.
* Escolha o tipo de algoritmo de inserção deseja usar (first-fit, best-fit, worst-fit).
* Digite quantos processos serão criados no decorrer do programa.
* Digite o tamanho (S) máximo de um processo (valores randômicos serão gerados de 1 à S).
* Estabeleça um limite (X) para o intervalo de valores válidos para definir prioridades de processos (valores randômicos serão gerados de 0 à X).
* Forneça o tempo máximo (T), em segundos, para o intervalo entre dois processos (tempos randômicos serão gerados de 0 à T).
* Pressione o botão de inicialização e veja. 

### Carregando um arquivo
* Escolha, entre as opções de execução, carregar um arquivo:
	O arquivo deve ter o caminho relativo ao diretório raiz.
* Se o arquivo for válido, o programa iniciará automaticamente, basta pressionar o botão de inicialização.

