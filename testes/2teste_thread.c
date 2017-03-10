#include "../include/cthread.h"
#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>


void* func0(void *arg);
void* func1(void *arg);
void* func2(void *arg);
void* func3(void *arg);

int id0, id1, id2, id3;

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

	printf("ID0 - yielding\n");
	cyield();

	printf("ID0 - Finishing thread\n");

	return NULL;
}

void* func1(void *arg) {

	printf("ID1 - Starded thread\n");

	printf("ID1 - Waiting for ID0\n");	
	if(cjoin(id0) != 0){

		printf("cjoin denied\n");
	}

	printf("ID1 - Finishing thread\n");
	
	return NULL;
}

void* func2(void *arg) {

	printf("ID2 - Started thread\n");

	printf("ID2 - Waiting for ID0\n");
	if(cjoin(id1) != 0){

		printf("cjoin denied\n");
	}

	printf("ID2 - Finishing thread\n");
	
	return NULL;
}

void* func3(void *arg) {

	printf("ID3 - Started thread\n");

	printf("ID3 - Finishing thread\n");
	
	return NULL;
}
