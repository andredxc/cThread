/*	Implementação da biblioteca de thread cthread
*	
*	André Dexheimer Carneiro
*	00243653
*/

#include "../include/fila2.h"
#include "../include/cdata.h"
#include "../include/cthread.h"
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>


extern FILA2 eApto, eExec, eBloq;		//filas globais que indicam os estados das threads
extern int g_curTID, g_flagScheadContext;
extern ucontext_t *g_endOfThread, g_curContext, g_dispatcherContext;
extern ucontext_t g_schealuderContext;


/*-------------------------------------------------------------------
Função:	Cria a thread e a coloca na fila dos aptos
Ret:	TID da thread criada se deu certo
		<0 se ocorreu algum erro 
-------------------------------------------------------------------*/
int ccreate (void* (*start)(void*), void *arg){

	ucontext_t newThreadCo, mainContext;
	TCB_t *TCB_element, *TCB_main;

	if(g_curTID == 0){	//primeira thread criada pelo pragrama
		
		g_flagScheadContext = 1;
		TCB_main = (TCB_t*)malloc(sizeof(TCB_t));

		if(CreateFila2(&eExec) != 0){

			return -1;
		}
		if(CreateFila2(&eApto) != 0){

			return -1;
		}
		if(CreateFila2(&eBloq) != 0){

			return -1;
		}
		if(CreateFila2(&eDependencies) != 0){

			return -1;
		}

		TCB_main->state = PROCST_CRIACAO;
		TCB_main->tid = 0;
		g_curTID = 1;
		getcontext(&mainContext);
		TCB_main->context = mainContext;

		if(AppendFila2(&eExec, (void*)TCB_main) != 0){

			return -1;
		}

		TCB_main->state = PROCST_EXEC;
		
	}

	//aloca o nodo que será adicionado à lista
	TCB_element = (TCB_t*) malloc(sizeof(TCB_t));

	getcontext(&newThreadCo);
	newThreadCo.uc_link = &g_schealuderContext;			//manda para o contexto do fim da execução
 	newThreadCo.uc_stack.ss_sp = malloc(MEGABYTE);
 	newThreadCo.uc_stack.ss_size = MEGABYTE;

 	//define a função que o contexto executará
	makecontext(&newThreadCo, (void (*)(void))start, 1, arg);	

	//preenche a estrutura da nova thread
	TCB_element->context = newThreadCo;
	TCB_element->state = PROCST_CRIACAO;
	TCB_element->tid = g_curTID;
	g_curTID++;

	//adiciona a estrutura ao final da fila dos aptos
	if(AppendFila2(&eApto, (void*)TCB_element) != 0){

		return -1;
	}
	TCB_element->state = PROCST_APTO;

	//printAllQueues();
	
	return TCB_element->tid;
}


/*-------------------------------------------------------------------
Função:	Libera voluntariamente a CPU
Ret:	==0 se foi executada corretamente
		<0 se ocorreu algum erro 
-------------------------------------------------------------------*/
int cyield(void){

	TCB_t *buffer;
	int flagChangeContext=1;

	//passa da fila EXEC para APTO
	if(FirstFila2(&eExec) != 0){		//aponta para o primeiro elemento de EXEC

		return -1;
	}

	buffer = (TCB_t*)GetAtIteratorFila2(&eExec);	//copia o elemento apontado

	if(buffer == NULL){

		return -1;
	}
			
	buffer->state = PROCST_APTO;		//redefine o estado

	if(AppendFila2(&eApto, (void*)buffer) != 0){	//salva o elemento na fila de APTOS

		return -1;
	}

	if(DeleteAtIteratorFila2(&eExec) != 0){			//deleta o elemento da fila de EXEC

		return -1;
	}

	getcontext(&buffer->context);

	if(flagChangeContext){

		flagChangeContext = 0;
		scheaduler(1);
	}

	return 0;
}

/*-------------------------------------------------------------------
Função:	Bloqueia a thread até que a thread especificada termine sua execução
Ret:	==0 se foi executada corretamente
		<0 se ocorreu algum erro 
-------------------------------------------------------------------*/
int cjoin(int tid){

	TCB_t *buffer;
	CDEP *buffer2;
	int flagValid, flagSaveContext = 1;

	flagValid = validateTID(tid);

	if(flagValid == 0){

		return -1;
	}
	//a thread esperada existe
	//a thread esperada é válida(não está sendo esperada por mais ninguém)
	buffer2 = (CDEP*)malloc(sizeof(CDEP));
	buffer = (TCB_t*)GetAtIteratorFila2(&eExec);

	buffer2->awaited_tid = tid;		//TID esperado pela thread
	buffer2->tid = 	buffer->tid;	//TID da thread que está esperando

	if(AppendFila2(&eDependencies, (void*)buffer2) != 0){

		return -1;
	}
	
	if(exec_bloq() < 0){

		return -1;
	}

	getcontext(&buffer->context);

	if(flagSaveContext){

		flagSaveContext = 0;
		scheaduler(1);
	}
	
	return 0;
}


/*-------------------------------------------------------------------
Função:	Inicializa uma variável do tipo csem_t
Ret:	==0 se foi executada corretamente
		<0 se ocorreu algum erro 
-------------------------------------------------------------------*/
int csem_init(csem_t *sem, int count){

	PFILA2 newQueue;

	newQueue = (PFILA2)malloc(sizeof(FILA2));

	if(CreateFila2(newQueue) != 0){			//cria a fila inicializando os ponteiros da mesma

		return -1;
	}

	sem->count = count;			//define o campo count
	sem->fila = newQueue;		//define a fila

	return 0;
}

/*-------------------------------------------------------------------
Função:	Realiza o protocolo de entrada a uma área crítica
Ret:	==0 se foi executada corretamente
		<0 se ocorreu algum erro 
-------------------------------------------------------------------*/
int cwait(csem_t *sem){

	TCB_t *buffer;
	int flagSaveContext = 1;

	if(sem->count > 0){
	//o recurso está LIBERADO
		sem->count--;
	}
	else{
	//o recurso está BLOQUEADO
		sem->count--;

		FirstFila2(&eExec);
		buffer = (TCB_t*)GetAtIteratorFila2(&eExec);

		AppendFila2(sem->fila, (void*)buffer);		//adiciona a thread atual a fila de bloqueados pelo semáforos

		buffer->state = PROCST_BLOQ;
		getcontext(&buffer->context);

		//printQueue(sem->fila, "SEMAFORO");
		//printAllQueues();

		if(flagSaveContext){

			flagSaveContext = 0;
			if(exec_bloq() != 0){		//passa a thread para a fila BLOQ

				return -1;
			}
			scheaduler(1);
			//printQueue(sem->fila, "SEMAFORO");
			//printAllQueues();
		}

		
	}

	return 0;
}


/*-------------------------------------------------------------------
Função:	Realiza o protocolo de seída de uma área crítica
Ret:	==0 se foi executada corretamente
		<0 se ocorreu algum erro 
-------------------------------------------------------------------*/
int csignal(csem_t *sem){

	TCB_t *bufferSem;
	int tid_procurado;

	sem->count++;	//incremente o controle do semáforo

	FirstFila2(sem->fila);
	
	bufferSem = (TCB_t*)GetAtIteratorFila2(sem->fila);		

	if(bufferSem != NULL){
	//se há threads bloquadas no semáforo

		tid_procurado = bufferSem->tid;			//define o TID da thread que deve passar para APTO

		if(DeleteAtIteratorFila2(sem->fila) != 0){

			return -1;
		}

		if(bloq_apto(tid_procurado)){		//passa para a lista dos APTOS o elemento com o TID procurado

			return -1;
		}

		//printAllQueues();
	}

	return 0;
}


//---------------------------------------------------------------------------------OUTRAS------------------------------------------------------------

/*-------------------------------------------------------------------
Função:	Faz as trocas das threads entre os estados
Ret:	-----

flagEmpty:	= 1 se a fila EXEC está vazia
			= 0 se não está vazia 
-------------------------------------------------------------------*/
int scheaduler(int flagEmpty){

	TCB_t *buffer;
	CDEP *buffer2;
	int flagFound = 0, tid_finalizado, tid_procurado;

	if(g_flagScheadContext){

		g_flagScheadContext = 0;

		getcontext(&g_schealuderContext);
		g_schealuderContext.uc_link = 0;			//manda para o contexto do fim da execução
	 	g_schealuderContext.uc_stack.ss_sp = malloc(MEGABYTE);
 		g_schealuderContext.uc_stack.ss_size = MEGABYTE;
 		makecontext(&g_schealuderContext, (void (*)(void))scheaduler, 1, (void*)0);
	}
	
	if(!flagEmpty){
	//quando a fila EXEC não estiver limpa -> LOGO QUE A THREAD FINALIZA
		FirstFila2(&eExec);
		FirstFila2(&eDependencies);

		buffer = (TCB_t*)GetAtIteratorFila2(&eExec);

		tid_finalizado = buffer->tid;		//salva o TID da thread finalizada

		while(eDependencies.it != NULL && flagFound == 0){
		//percorre a lista de dependências procurando por alguma

			buffer2 = (CDEP*)GetAtIteratorFila2(&eDependencies);

			if(tid_finalizado == buffer2->awaited_tid){

				flagFound = 1;
				tid_procurado = buffer2->tid;	//o tid procurado é o da thread que possuia a dependência
			}
			else{

				NextFila2(&eDependencies);
			}
		}

		if(flagFound == 1){
		//achou alguma dependência com a thread finalizada
			bloq_apto(tid_procurado);
			DeleteAtIteratorFila2(&eDependencies);	//deleta a dependência depois que ela foi tratada
		}

		DeleteAtIteratorFila2(&eExec);
	}


	FirstFila2(&eApto);

	if(eApto.it != NULL){

		apto_exec();
	}
	else{

		exit(0);
	}	

	return 0;
}







/*-------------------------------------------------------------------
Função:	Troca as filas APTO e EXEC e muda o contexto
Ret:	==0 se executou normalmente
		<0 se ocorreu algum erro
-------------------------------------------------------------------*/
int apto_exec(){

	TCB_t *buffer;

	//passa da fila APTO para EXEC----------------------------
	if(FirstFila2(&eApto) != 0){

		return -1;
	}

	buffer = (TCB_t*)GetAtIteratorFila2(&eApto);	//le o primeiro elemento do APTO

	if(buffer == NULL){

		return -1;
	}

	buffer->state = PROCST_EXEC;		//redefine o estado da thread

	if(AppendFila2(&eExec, (void*)buffer) != 0){			//coloca o elemento na fila de EXEC

		return -1;
	}	

	if(DeleteAtIteratorFila2(&eApto) != 0){		//deleta o elemento da fila dos APTOS

		return -1;
	}

	setcontext(&buffer->context);

	return 0;	
}



/*-------------------------------------------------------------------
Função:	Passa a thread do estado EXEC para o estado BLOQ
Ret:	==0 se executou corretamente
		<0 caso contrário
-------------------------------------------------------------------*/
int exec_bloq(){

	TCB_t *buffer;

	FirstFila2(&eExec);

	buffer = (TCB_t*)GetAtIteratorFila2(&eExec);	//le o primeiro elemento do EXEC

	if(buffer == NULL){

		return -1;
	}

	buffer->state = PROCST_BLOQ;		//redefine o estado da thread

	AppendFila2(&eBloq, (void*)buffer);		//coloca o elemento na fila de BLOQ

	if(DeleteAtIteratorFila2(&eExec) != 0){		//deleta o elemento da fila dos EXEC

		return -1;
	}

	return 0;
}


/*-------------------------------------------------------------------
Função:	Passa a thread requerida do estado BLOQ para APTO
Ret:	==0 se executou corretamente
		<0 caso contrário
-------------------------------------------------------------------*/
int bloq_apto(int tid_procurado){

	TCB_t *buffer;
	int flagFound = 0;

	FirstFila2(&eBloq);
			
	while(eBloq.it != NULL && flagFound == 0){
	//percorre a lista de bloqueados procurando pelo tid
		buffer = (TCB_t*)GetAtIteratorFila2(&eBloq);

		if(buffer->tid == tid_procurado){
		//achou o elemento que deve ser passado para APTO
			buffer->state = PROCST_APTO;
			if(AppendFila2(&eApto, (void*)buffer) != 0){

				return -1;
			}
			if(DeleteAtIteratorFila2(&eBloq) != 0){

				return -1;
			}
			flagFound = 1;
		}
		else{

			NextFila2(&eBloq);
		}
	}
	return 0;
}



/*-------------------------------------------------------------------
Função:	Descobre se o TID passado é válido como depenência
Ret:	== 1 se o TID é válido(existe e não é amarrado a nenhuma outra thread) 
		== 0 se o TID não é válido
-------------------------------------------------------------------*/
int validateTID(int tid){

	TCB_t *buffer;
	CDEP *buffer2;
	int flagFound = 0, flagValid = 1;

	if(tid < 0 || tid >= g_curTID){
	//verifica se está entre os limites
		return 0;
	}

	FirstFila2(&eExec);
	FirstFila2(&eApto);

	buffer = (TCB_t*)GetAtIteratorFila2(&eExec);
	if(buffer->tid == tid){
	//verifica se a thread esperada está na fila de EXEC
		flagFound = 1;
	}
	else{
		
		while(eApto.it != NULL && flagFound == 0){		
		//percorre APTOS para verificar a existência da thread especifidada
			buffer = (TCB_t*)GetAtIteratorFila2(&eApto);

			if(buffer->tid == tid){

				flagFound = 1;
			}
			else{

				NextFila2(&eApto);
			}
		}
	}

	if(flagFound == 0){			

		return 0;
	}
	//a thread existe

	FirstFila2(&eDependencies);

	while(eDependencies.it != NULL && flagValid == 1){
	//percorre a lista de dependências para verificar se é uma dependência válida
		buffer2 = (CDEP*)GetAtIteratorFila2(&eDependencies);

		if(buffer2->awaited_tid == tid){

			flagValid = 0;
		}
		else{

			NextFila2(&eDependencies);
		}
	}
	return flagValid;
}





//------------------------------------------------FUNÇÕES TEMPORÁRIAS DE TESTE--------------------------------

void printQueue(PFILA2 queue, char* queueName){

	TCB_t *buffer;

	int count = 0;

	FirstFila2(queue);

	printf("\n--Queue \"%s\"\n\n", queueName);

	while(queue->it != NULL){

		buffer = (TCB_t*)GetAtIteratorFila2(queue);

		printf("Element number: %d\n", count);
		printf("TID: %d\n", buffer->tid);
		printf("State: %d\n", buffer->state);
		//printf("Context: %d\n", queue->it->content->(int)context);

		NextFila2(queue);
		count++;
	}

	printf("\n--End of queue\n");
}

void printAllQueues(){

	printQueue(&eExec, "EXECUTANDO");
	printQueue(&eApto, "APTO");
	printQueue(&eBloq, "BLOQUEADO");
}

void printQueue2(PFILA2 queue, char* queueName){

	CDEP *buffer;

	int count = 0;

	FirstFila2(queue);

	printf("\n--Queue \"%s\"\n\n", queueName);

	while(queue->it != NULL){

		buffer = (CDEP*)GetAtIteratorFila2(queue);

		printf("Element number: %d\n", count);
		printf("TID: %d\n", buffer->tid);
		printf("Awaited TID: %d\n", buffer->awaited_tid);
		//printf("Context: %d\n", queue->it->content->(int)context);

		NextFila2(queue);
		count++;
	}

	printf("\n--End of queue\n");
}