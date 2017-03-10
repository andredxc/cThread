#include "../include/cthread.h"
#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>


void* func0(void *arg);
void* func1(void *arg);
void* func2(void *arg);
void* func3(void *arg);

int id0, id1, id2, id3;
csem_t semaforo;


int main(int argc, char *argv[]){

	int i=5;

	id0 = ccreate(func0, (void *)&i);
	printf("MAIN - created thread ID0\n");
	id1 = ccreate(func1, (void *)&i);
	printf("MAIN - created thread ID1\n");
	id2 = ccreate(func2, (void *)&i);
	printf("MAIN - created thread ID2\n");
	id3 = ccreate(func3, (void *)&i);
	printf("MAIN - created thread ID3\n");

	csem_init(&semaforo, 2);

	printf("MAIN - trying to access critical area\n");
	cwait(&semaforo);

	printf("MAIN - inside the critical area\n");
	printf("MAIN - waiting for ID3\n");
	if(cjoin(id3) != 0){

		printf("cjoin denied\n");
	}
	printf("MAIN - leaving the critical area\n");

	csignal(&semaforo);

	printf("MAIN - Waiting for ID1\n");
	if(cjoin(id1) != 0){

		printf("cjoin denied\n");
	}

	
	
	printf("id0 = %d\n", id0);
	printf("id1 = %d\n", id1);
	printf("id2 = %d\n", id2);
	printf("id3 = %d\n", id3);


	printf("END OF MAIN FUNCTION\n");
	return 0;
}

void* func0(void *arg) {
	
	printf("ID0 - Starded thread\n");
	printf("ID0 - trying to acces critical area\n");
	cwait(&semaforo);

	printf("ID0 - inside the critical area\n");
	printf("ID0 - yielding\n");
	cyield();
	printf("ID0 - leaving the critical area\n");
	
	csignal(&semaforo);
	printf("ID0 - Finishing thread\n");

	return NULL;
}

void* func1(void *arg) {

	printf("ID1 - Starded thread\n");

	printf("ID1 - trying to acces critical area\n");
	cwait(&semaforo);

	printf("ID1 - inside the critical area\n");
	printf("ID1 - yielding\n");
	cyield();
	printf("ID1 - leaving the critical area\n");
	
	csignal(&semaforo);	
	if(cjoin(id0) != 0){

		printf("cjoin denied\n");
	}

	printf("ID1 - Finishing thread\n");
	
	return NULL;
}

void* func2(void *arg) {

	printf("ID2 - Started thread\n");

	printf("ID2 - Waiting for ID3\n");
	if(cjoin(id3) != 0){

		printf("cjoin denied\n");
	}

	printf("ID2 - Finishing thread\n");
	
	return NULL;
}

void* func3(void *arg) {

	printf("ID3 - Started thread\n");

	printf("ID3 - Waiting for TID = 5\n");
	if(cjoin(5) != 0){

		printf("cjoin denied\n");
	}

	printf("ID3 - Finishing thread\n");
	
	return NULL;
}
