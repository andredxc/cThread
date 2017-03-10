#include "../include/cthread.h"
#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>


void* func0(void *arg);
void* func1(void *arg);
void* func2(void *arg);



int main(int argc, char *argv[]){

	int id0, id1, id2;
	int arg0 = 25, arg1 = 35, arg2 = 45;


	id0 = ccreate(func0, (void *)&arg0);
	printf("MAIN - created thread ID0\n");
	id1 = ccreate(func1, (void *)&arg1);
	printf("MAIN - created thread ID1\n");

	cyield();

	id2 = ccreate(func2, (void *)&arg2);
	printf("MAIN - created thread ID2\n");
	
	cyield();

	printf("id0 TID = %d\n", id0);
	printf("id1 TID = %d\n", id1);
	printf("id2 TID = %d\n", id2);

	printf("END OF MAIN FUNCTION\n");
	return 0;
}

void* func0(void *arg) {
	
	printf("Thread ID0 - printing %d\n", *((int *)arg));

	printf("Thread ID0 - finishing thread\n");
	return NULL;
}

void* func1(void *arg) {
	
	printf("Thread ID1 - printing %d\n", *((int *)arg));

	printf("Thread ID1 - finishing thread\n");
	return NULL;
}

void* func2(void *arg) {
	printf("Thread ID2 - printing %d\n", *((int *)arg));

	printf("Thread ID2 - finishing thread\n");
	return NULL;
}
