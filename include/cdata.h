/*
 * André Dexheimer Carneiro
 * 00243653
 */

#include "../include/fila2.h"

#include <ucontext.h>

#ifndef __cdata__
#define __cdata__

#define	PROCST_CRIACAO	0
#define	PROCST_APTO	1
#define	PROCST_EXEC	2
#define	PROCST_BLOQ	3
#define	PROCST_TERMINO	4

/* NÃO ALTERAR ESSA struct */
typedef struct s_TCB { 
	int		tid; 		// identificador da thread
	int		state;		// estado em que a thread se encontra
						// 0: Criação; 1: Apto; 2: Execução; 3: Bloqueado e 4: Término
	ucontext_t 	context;	// contexto de execução da thread (SP, PC, GPRs e recursos) 
} TCB_t;

typedef struct cjoin_dep{
	int tid;			//tid da thread que esta esperando
	int awaited_tid;	//tid da thread sendo esperada
} CDEP;

FILA2 eApto, eExec, eBloq;		//filas globais que indicam os estados das threads
FILA2 eDependencies;			//fila que contém as dependências das threads em estado bloqueado
int g_flagScheadContext;		//flag que diz se o contexto do escalonador já foi definido
int g_curTID;					//informa o TID da próxima thread a ser criada
ucontext_t g_dispatcherContext, g_curContext, g_schealuderContext;
ucontext_t *g_endOfThread;

int scheaduler();
void initStructure();
int apto_exec();
int exec_bloq();
int bloq_apto();
int validateTID(int tid);

void printQueue();
void printAllQueues();
void printQueue2();

#endif